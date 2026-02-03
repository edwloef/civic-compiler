%{

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "ccn/ccn.h"
#include "error/error.h"
#include "globals/globals.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "parser.h"
#include "utils.h"

int yylex();

static node_st *parseresult = NULL;
extern FILE *yyin;

node_st *rev_vardecls(node_st *root);
YYLTYPE span_locs(YYLTYPE lhs, YYLTYPE rhs);
void add_loc_to_node(node_st *node, YYLTYPE loc);
void yyerror(char const *errname);

%}

%union {
    bool cbool;
    int cint;
    double cfloat;
    char *cstr;
    enum BasicType basic_type;
    node_st *node;
}

%define parse.error verbose
%define parse.trace
%locations

%token PAREN_L "("
%token PAREN_R ")"
%token BRACKET_L "["
%token BRACKET_R "]"
%token BRACE_L "{"
%token BRACE_R "}"

%token COMMA ","
%token SEMICOLON ";"

%token BANG "!"
%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token SLASH "/"
%token PERCENT "%"
%token LE "<="
%token LT "<"
%token GE ">="
%token GT ">"
%token EQ "=="
%token NE "!="
%token AND "&&"
%token OR "||"
%token ASSIGN "="

%token TY_INT "int"
%token TY_FLOAT "float"
%token TY_BOOL "bool"
%token TY_VOID "void"

%token KW_EXTERN "extern"
%token KW_EXPORT "export"
%token KW_IF "if"
%token KW_ELSE "else"
%token KW_WHILE "while"
%token KW_DO "do"
%token KW_FOR "for"
%token KW_RETURN "return"

%token <cbool> LIT_BOOL "boolean literal"
%token <cint> LIT_INT "integer literal"
%token <cfloat> LIT_FLOAT "floating point literal"
%token <cstr> ID "identifier"

%type <node> decls stmts exprs arrexprs ids params
%type <node> program decl stmt expr block varref id
%type <node> vardecl vardecls fundef fundefs funheader
%type <basic_type> basictype

%precedence "none"
%precedence "else"
%left "||"
%left "&&"
%left "==" "!="
%left "<" "<=" ">" ">="
%left "+" "-"
%left "*" "/" "%"
%precedence "monop"

%start program

%%

program: decls
         {
           parseresult = ASTprogram($1);
         }
       ;

decls: decl decls
       {
         $$ = ASTdecls($1, $2);
       }
     | decl
       {
         $$ = ASTdecls($1, NULL);
       }
     | decl stmt
       {
        yylloc = @2;
        yyerror("unexpected statement, expected declaration");
       }
     ;

decl: "extern" funheader ";"
      {
        $$ = $2;
        FUNDECL_EXTERNAL($$) = true;
      }
    | "extern" basictype id ";"
      {
        $$ = ASTvardecl(ASTtype(NULL, $2, false), $3, NULL);
        VARDECL_EXTERNAL($$) = true;
        VARDECL_GLOBAL($$) = true;
      }
    | "extern" basictype "[" ids "]" id ";"
      {
        $$ = ASTvardecl(ASTtype($4, $2, true), $6, NULL);
        VARDECL_EXTERNAL($$) = true;
        VARDECL_GLOBAL($$) = true;
      }
    | "export" fundef
      {
        $$ = $2;
        FUNDECL_EXPORTED($$) = true;
      }
    | "export" vardecl
      {
        $$ = $2;
        VARDECL_EXPORTED($$) = true;
        VARDECL_GLOBAL($$) = true;
      }
    | fundef
      {
        $$ = $1;
      }
    | vardecl
      {
        $$ = $1;
        VARDECL_GLOBAL($$) = true;
      }
    ;

stmts: stmt stmts
       {
         $$ = ASTstmts($1, $2);
         add_loc_to_node($$, @$);
       }
     | stmt
       {
         $$ = ASTstmts($1, NULL);
         add_loc_to_node($$, @$);
       }
     | stmt fundef
       {
         yylloc = @2;
         yyerror("unexpected function definition, expected statement");
       }
     | stmt vardecl
       {
         yylloc = @2;
         yyerror("unexpected variable declaration, expected statement");
       }
     ;

stmt: expr ";"
      {
        if (NODE_TYPE($1) == NT_CALL) {
            $$ = $1;
        } else {
            yylloc = @1;
            yyerror("unexpected expression, expected statement");
        }
      }
    | varref "=" expr ";"
      {
        VARREF_WRITE($1) = true;
        $$ = ASTassign($1, $3);
        @$ = span_locs(@1, @3);
        add_loc_to_node($$, @$);
      }
    | "if" "(" expr ")" block %prec "none"
      {
        $$ = ASTifelse($3, $5, NULL);
        add_loc_to_node($$, @$);
      }
    | "if" "(" expr ")" block "else" block
      {
        $$ = ASTifelse($3, $5, $7);
        add_loc_to_node($$, @$);
      }
    | "while" "(" expr ")" block
      {
        $$ = ASTwhile($3, $5);
        add_loc_to_node($$, @$);
      }
    | "do" block "while" "(" expr ")" ";"
      {
        $$ = ASTdowhile($2, $5);
        @$ = span_locs(@1, @6);
        add_loc_to_node($$, @$);
      }
    | "for" "(" "int" id "=" expr "," expr ")" block
      {
        $$ = ASTfor(ASTvarref($4, NULL), $6, $8, ASTint(1, TY_int), $10);
        add_loc_to_node(FOR_REF($$), @4);
        add_loc_to_node($$, @$);
      }
    | "for" "(" "int" id "=" expr "," expr "," expr ")" block
      {
        $$ = ASTfor(ASTvarref($4, NULL), $6, $8, $10, $12);
        add_loc_to_node(FOR_REF($$), @4);
        add_loc_to_node($$, @$);
      }
    | "return" ";"
      {
        $$ = ASTreturn(NULL);
        add_loc_to_node($$, @1);
      }
    | "return" expr ";"
      {
        $$ = ASTreturn($2);
        @$ = span_locs(@1, @2);
        add_loc_to_node($$, @$);
      }
    ;

exprs: expr "," exprs
       {
         $$ = ASTexprs($1, $3);
       }
     | expr
       {
         $$ = ASTexprs($1, NULL);
       }
     ;

expr: "(" basictype ")" expr %prec "monop"
      {
        $$ = ASTcast($4, $2);
        add_loc_to_node($$, @$);
      }
    | "+" expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_pos, 0);
        add_loc_to_node($$, @$);
      }
    | "-" expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_neg, 0);
        add_loc_to_node($$, @$);
      }
    | "!" expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_not, 0);
        add_loc_to_node($$, @$);
      }
    | expr "*" expr
      {
        $$ = ASTbinop($1, $3, BO_mul, 0);
        add_loc_to_node($$, @$);
      }
    | expr "/" expr
      {
        $$ = ASTbinop($1, $3, BO_div, 0);
        add_loc_to_node($$, @$);
      }
    | expr "%" expr
      {
        $$ = ASTbinop($1, $3, BO_mod, 0);
        add_loc_to_node($$, @$);
      }
    | expr "+" expr
      {
        $$ = ASTbinop($1, $3, BO_add, 0);
        add_loc_to_node($$, @$);
      }
    | expr "-" expr
      {
        $$ = ASTbinop($1, $3, BO_sub, 0);
        add_loc_to_node($$, @$);
      }
    | expr "<" expr
      {
        $$ = ASTbinop($1, $3, BO_lt, 0);
        add_loc_to_node($$, @$);
      }
    | expr "<=" expr
      {
        $$ = ASTbinop($1, $3, BO_le, 0);
        add_loc_to_node($$, @$);
      }
    | expr ">" expr
      {
        $$ = ASTbinop($1, $3, BO_gt, 0);
        add_loc_to_node($$, @$);
      }
    | expr ">=" expr
      {
        $$ = ASTbinop($1, $3, BO_ge, 0);
        add_loc_to_node($$, @$);
      }
    | expr "==" expr
      {
        $$ = ASTbinop($1, $3, BO_eq, 0);
        add_loc_to_node($$, @$);
      }
    | expr "!=" expr
      {
        $$ = ASTbinop($1, $3, BO_ne, 0);
        add_loc_to_node($$, @$);
      }
    | expr "&&" expr
      {
        $$ = ASTbinop($1, $3, BO_and, 0);
        add_loc_to_node($$, @$);
      }
    | expr "||" expr
      {
        $$ = ASTbinop($1, $3, BO_or, 0);
        add_loc_to_node($$, @$);
      }
    | "(" expr ")"
      {
        $$ = $2;
        add_loc_to_node($$, @$);
      }
    | id "(" exprs ")"
      {
        $$ = ASTcall($1, $3);
        add_loc_to_node($$, @$);
      }
    | id "(" ")"
      {
        $$ = ASTcall($1, NULL);
        add_loc_to_node($$, @$);
      }
    | varref
      {
        $$ = $1;
        add_loc_to_node($$, @$);
      }
    | "integer literal"
      {
        $$ = ASTint($1, TY_int);
        add_loc_to_node($$, @$);
      }
    | "floating point literal"
      {
        $$ = ASTfloat($1, TY_float);
        add_loc_to_node($$, @$);
      }
    | "boolean literal"
      {
        $$ = ASTbool($1, TY_bool);
        add_loc_to_node($$, @$);
      }
    | "[" arrexprs "]"
      {
        $$ = $2;
        add_loc_to_node($$, @$);
      }
    | "[" "]"
      {
        yylloc = @$;
        yyerror("array literals must be non-empty");
      }
    ;

arrexprs: expr "," arrexprs
          {
            $$ = ASTarrexprs($1, $3);
            add_loc_to_node($$, @$);
          }
        | expr
          {
            $$ = ASTarrexprs($1, NULL);
            add_loc_to_node($$, @$);
          }
        ;

vardecls: vardecls vardecl
          {
            $$ = ASTdecls($2, $1);
          }
        | vardecl
          {
            $$ = ASTdecls($1, NULL);
          }
        ;

vardecl: basictype id ";"
         {
           $$ = ASTvardecl(ASTtype(NULL, $1, false), $2, NULL);
         }
       | basictype "[" exprs "]" id ";"
         {
           $$ = ASTvardecl(ASTtype($3, $1, true), $5, NULL);
         }
       | basictype id "=" expr ";"
         {
           $$ = ASTvardecl(ASTtype(NULL, $1, false), $2, $4);
           @$ = span_locs(@1, @4);
           add_loc_to_node($$, @$);
         }
       | basictype "[" exprs "]" id "=" expr ";"
         {
           $$ = ASTvardecl(ASTtype($3, $1, true), $5, $7);
           @$ = span_locs(@1, @7);
           add_loc_to_node($$, @$);
         }
       ;

fundefs: fundef fundefs
         {
           $$ = ASTdecls($1, $2);
         }
       | fundef
         {
           $$ = ASTdecls($1, NULL);
         }
       | fundef vardecl
         {
           yylloc = @2;
           yyerror("unexpected variable declaration, expected function definition or statement");
         }
       ;

fundef: funheader ";"
        {
          yylloc = @1;
          yyerror("unexpected function declaration, expected function definition");
        }
      | funheader "{" "}"
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, NULL);
        }
      | funheader "{" vardecls "}"
        {
          $3 = rev_vardecls($3);

          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, NULL);
        }
      | funheader "{" fundefs "}"
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, NULL);
        }
      | funheader "{" stmts "}"
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, $3);
        }
      | funheader "{" vardecls fundefs "}"
        {
          $3 = rev_vardecls($3);

          node_st *vardecl;
          for (vardecl = $3; DECLS_NEXT(vardecl); vardecl = DECLS_NEXT(vardecl));
          DECLS_NEXT(vardecl) = $4;

          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, NULL);
        }
      | funheader "{" vardecls stmts "}"
        {
          $3 = rev_vardecls($3);

          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, $4);
        }
      | funheader "{" fundefs stmts "}"
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, $4);
        }
      | funheader "{" vardecls fundefs stmts "}"
        {
          $3 = rev_vardecls($3);

          node_st *vardecl;
          for (vardecl = $3; DECLS_NEXT(vardecl); vardecl = DECLS_NEXT(vardecl));
          DECLS_NEXT(vardecl) = $4;

          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, $5);
        }
      ;

funheader: basictype id "(" params ")"
           {
             $$ = ASTfundecl($2, $4, $1);
           }
         | "void" id "(" params ")"
           {
             $$ = ASTfundecl($2, $4, TY_void);
           }
         | basictype id "(" ")"
           {
             $$ = ASTfundecl($2, NULL, $1);
           }
         | "void" id "(" ")"
           {
             $$ = ASTfundecl($2, NULL, TY_void);
           }
         ;

block: stmt
       {
         $$ = ASTstmts($1, NULL);
       }
     | "{" stmts "}"
       {
         $$ = $2;
       }
     | "{" "}"
       {
         $$ = NULL;
       }
     ;

varref: id "[" exprs "]"
        {
          $$ = ASTvarref($1, $3);
          add_loc_to_node($$, @$);
        }
      | id
        {
          $$ = ASTvarref($1, NULL);
          add_loc_to_node($$, @$);
        }
      ;

id: "identifier"
    {
      $$ = ASTid($1);
      add_loc_to_node($$, @$);
    }
  ;

params: basictype id "," params
        {
          $$ = ASTparams(ASTparam(ASTtype(NULL, $1, false), $2), $4);
        }
      | basictype "[" ids "]" id "," params
        {
          $$ = ASTparams(ASTparam(ASTtype($3, $1, true), $5), $7);
        }
      | basictype id
        {
          $$ = ASTparams(ASTparam(ASTtype(NULL, $1, false), $2), NULL);
        }
      | basictype "[" ids "]" id
        {
          $$ = ASTparams(ASTparam(ASTtype($3, $1, true), $5), NULL);
        }
      ;

ids: id "," ids
     {
       $$ = ASTexprs(ASTvarref($1, NULL), $3);
       add_loc_to_node(EXPRS_EXPR($$), @1);
     }
   | id
     {
       $$ = ASTexprs(ASTvarref($1, NULL), NULL);
       add_loc_to_node(EXPRS_EXPR($$), @1);
     }
   ;

basictype: "bool"
           {
             $$ = TY_bool;
           }
         | "int"
           {
             $$ = TY_int;
           }
         | "float"
           {
             $$ = TY_float;
           }
         ;

%%

node_st *rev_vardecls(node_st *root) {
    node_st *curr = root;
    node_st *prev = NULL;

    while (curr) {
        node_st *next = DECLS_NEXT(curr);
        DECLS_NEXT(curr) = prev;
        prev = curr;
        curr = next;
    }

    return prev;
}

YYLTYPE span_locs(YYLTYPE lhs, YYLTYPE rhs) {
    YYLTYPE loc = {lhs.first_line, lhs.first_column, rhs.last_line, rhs.last_column};
    return loc;
}

void add_loc_to_node(node_st *node, YYLTYPE loc) {
    NODE_BLINE(node) = loc.first_line;
    NODE_BCOL(node) = loc.first_column;
    NODE_ELINE(node) = loc.last_line;
    NODE_ECOL(node) = loc.last_column;
    if (!NODE_FILENAME(node))
        NODE_FILENAME(node) = STRcpy(globals.file);
}

int cpp_status = 0;

void yyerror(char const *error) {
    if (cpp_status) {
        exit(cpp_status);
    }

    span s = {yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, globals.file};
    emit_message_with_span(s, L_ERROR, "%s", error);
    abort_on_error();
}

void sigchld_handler(int sig, siginfo_t *si, void *context) {
    (void)sig;
    (void)context;

    cpp_status = si->si_status;
}

node_st *scanparse(node_st *root) {
    (void)root;

    DBUG_ASSERT(root == NULL, "Started parsing with existing syntax tree.");

    if (globals.input_file) {
        yyin = fopen(globals.input_file, "r");
        if (yyin == NULL) {
            emit_message(L_ERROR, "couldn't open '%s': %s (os error %d)", globals.input_file, strerror(errno), errno);
            abort_on_error();
        }
    } else {
        yyin = stdin;
    }

    if (globals.preprocessor) {
        if (globals.input_file) {
            fclose(yyin);
            yyin = NULL;
        }

        struct sigaction sa;
        sa.sa_sigaction = sigchld_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART|SA_SIGINFO|SA_NOCLDWAIT;
        sigaction(SIGCHLD, &sa, NULL);

        int static_argv_count = 3;
        char *argv[] = {"cpp", "-traditional-cpp", "-nostdinc", NULL, NULL, NULL};

        if (globals.input_file) {
            char *file_name = globals.input_file;
            if (file_name[0] == '-') {
                file_name = STRfmt("./%s", file_name);
            }
            argv[static_argv_count] = file_name;
        } else {
            char *cwd = getcwd(NULL, 0);
            if (cwd) {
                argv[static_argv_count] = "-I";
                argv[static_argv_count + 1] = cwd;
            } else {
                emit_message(L_WARNING, "couldn't get current working directory: %s (os error %d)", strerror(errno), errno);
            }
        }

        yyin = spawn_command(yyin, "cpp", argv);

        if (globals.input_file) {
            if (argv[static_argv_count] != globals.input_file) {
                MEMfree(argv[static_argv_count]);
            }
        } else {
            MEMfree(argv[static_argv_count + 1]);
        }

        if (!yyin) {
            emit_message(L_ERROR, "couldn't start c preprocessor: %s (os error %d)", strerror(errno), errno);
            abort_on_error();
        }
    }

    if (!globals.input_file) {
        globals.input_file = STRcpy("<stdin>");
    }

    globals.file = STRcpy(globals.input_file);

    yyparse();

    if (cpp_status) {
        exit(cpp_status);
    }

    fclose(yyin);
    MEMfree(globals.file);
    globals.file = NULL;

    return parseresult;
}
