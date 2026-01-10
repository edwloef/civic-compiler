%{

#include <errno.h>
#include <string.h>

#include "analysis/error.h"
#include "ccn/ccn.h"
#include "globals/globals.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "parser.h"

int yylex();

static node_st *parseresult = NULL;
extern FILE *yyin;

node_st *rev_vardecls(node_st *root);
YYLTYPE span_locs(YYLTYPE lhs, YYLTYPE rhs);
void add_loc_to_node(node_st *node, YYLTYPE loc);
void yyerror(char *errname);

%}

%union {
    bool cbool;
    int cint;
    float cfloat;
    char *cstr;
    enum BasicType basic_type;
    node_st *node;
}

%define parse.trace
%locations

%token PAREN_L PAREN_R BRACKET_L BRACKET_R BRACE_L BRACE_R COMMA SEMICOLON
%token BANG PLUS MINUS STAR SLASH PERCENT LE LT GE GT EQ NE AND OR ASSIGN
%token KW_EXTERN KW_EXPORT KW_IF KW_ELSE KW_WHILE KW_DO KW_FOR KW_RETURN
%token TY_INT TY_FLOAT TY_BOOL TY_VOID

%token <cbool> LIT_BOOL
%token <cint> LIT_INT
%token <cfloat> LIT_FLOAT
%token <cstr> ID

%type <node> decls stmts exprs arrexprs ids params
%type <node> program decl stmt expr arrexpr block varref id
%type <node> vardecl vardecls fundef fundefs funheader
%type <basic_type> basictype

%precedence "none"
%precedence KW_ELSE
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left STAR SLASH PERCENT
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
     ;

decl: KW_EXTERN funheader SEMICOLON
      {
        $$ = $2;
        FUNDECL_EXTERNAL($$) = true;
      }
    | KW_EXTERN basictype id SEMICOLON
      {
        $$ = ASTvardecl(ASTtype(NULL, $2), $3, NULL);
        VARDECL_EXTERNAL($$) = true;
        VARDECL_GLOBAL($$) = true;
      }
    | KW_EXTERN basictype BRACKET_L ids BRACKET_R id SEMICOLON
      {
        $$ = ASTvardecl(ASTtype($4, $2), $6, NULL);
        VARDECL_EXTERNAL($$) = true;
        VARDECL_GLOBAL($$) = true;
      }
    | KW_EXPORT fundef
      {
        $$ = $2;
        FUNDECL_EXPORTED($$) = true;
      }
    | KW_EXPORT vardecl
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
     ;

stmt: id PAREN_L exprs PAREN_R SEMICOLON
      {
        $$ = ASTcall($1, $3);
        @$ = span_locs(@1, @4);
        add_loc_to_node($$, @$);
      }
    | id PAREN_L PAREN_R SEMICOLON
      {
        $$ = ASTcall($1, NULL);
        @$ = span_locs(@1, @3);
        add_loc_to_node($$, @$);
      }
    | varref ASSIGN arrexpr SEMICOLON
      {
        $$ = ASTassign($1, $3);
        @$ = span_locs(@1, @3);
        add_loc_to_node($$, @$);
      }
    | KW_IF PAREN_L expr PAREN_R block %prec "none"
      {
        $$ = ASTifelse($3, $5, NULL);
        add_loc_to_node($$, @$);
      }
    | KW_IF PAREN_L expr PAREN_R block KW_ELSE block
      {
        $$ = ASTifelse($3, $5, $7);
        add_loc_to_node($$, @$);
      }
    | KW_WHILE PAREN_L expr PAREN_R block
      {
        $$ = ASTwhile($3, $5);
        add_loc_to_node($$, @$);
      }
    | KW_DO block KW_WHILE PAREN_L expr PAREN_R SEMICOLON
      {
        $$ = ASTdowhile($2, $5);
        @$ = span_locs(@1, @6);
        add_loc_to_node($$, @$);
      }
    | KW_FOR PAREN_L TY_INT id ASSIGN expr COMMA expr PAREN_R block
      {
        $$ = ASTfor($4, $6, $8, ASTint(1), $10);
        add_loc_to_node($$, @$);
      }
    | KW_FOR PAREN_L TY_INT id ASSIGN expr COMMA expr COMMA expr PAREN_R block
      {
        $$ = ASTfor($4, $6, $8, $10, $12);
        add_loc_to_node($$, @$);
      }
    | KW_RETURN SEMICOLON
      {
        $$ = ASTreturn(NULL);
        add_loc_to_node($$, @1);
      }
    | KW_RETURN expr SEMICOLON
      {
        $$ = ASTreturn($2);
        @$ = span_locs(@1, @2);
        add_loc_to_node($$, @$);
      }
    ;

exprs: expr COMMA exprs
       {
         $$ = ASTexprs($1, $3);
       }
     | expr
       {
         $$ = ASTexprs($1, NULL);
       }
     ;

expr: PAREN_L basictype PAREN_R expr %prec "monop"
      {
        $$ = ASTcast($4, $2);
        add_loc_to_node($$, @$);
      }
    | PLUS expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_pos);
        add_loc_to_node($$, @$);
      }
    | MINUS expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_neg);
        add_loc_to_node($$, @$);
      }
    | BANG expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_not);
        add_loc_to_node($$, @$);
      }
    | expr STAR expr
      {
        $$ = ASTbinop($1, $3, BO_mul);
        add_loc_to_node($$, @$);
      }
    | expr SLASH expr
      {
        $$ = ASTbinop($1, $3, BO_div);
        add_loc_to_node($$, @$);
      }
    | expr PERCENT expr
      {
        $$ = ASTbinop($1, $3, BO_mod);
        add_loc_to_node($$, @$);
      }
    | expr PLUS expr
      {
        $$ = ASTbinop($1, $3, BO_add);
        add_loc_to_node($$, @$);
      }
    | expr MINUS expr
      {
        $$ = ASTbinop($1, $3, BO_sub);
        add_loc_to_node($$, @$);
      }
    | expr LT expr
      {
        $$ = ASTbinop($1, $3, BO_lt);
        add_loc_to_node($$, @$);
      }
    | expr LE expr
      {
        $$ = ASTbinop($1, $3, BO_le);
        add_loc_to_node($$, @$);
      }
    | expr GT expr
      {
        $$ = ASTbinop($1, $3, BO_gt);
        add_loc_to_node($$, @$);
      }
    | expr GE expr
      {
        $$ = ASTbinop($1, $3, BO_ge);
        add_loc_to_node($$, @$);
      }
    | expr EQ expr
      {
        $$ = ASTbinop($1, $3, BO_eq);
        add_loc_to_node($$, @$);
      }
    | expr NE expr
      {
        $$ = ASTbinop($1, $3, BO_ne);
        add_loc_to_node($$, @$);
      }
    | expr AND expr
      {
        $$ = ASTbinop($1, $3, BO_and);
        add_loc_to_node($$, @$);
      }
    | expr OR expr
      {
        $$ = ASTbinop($1, $3, BO_or);
        add_loc_to_node($$, @$);
      }
    | PAREN_L expr PAREN_R
      {
        $$ = $2;
        add_loc_to_node($$, @$);
      }
    | id PAREN_L exprs PAREN_R
      {
        $$ = ASTcall($1, $3);
        add_loc_to_node($$, @$);
      }
    | id PAREN_L PAREN_R
      {
        $$ = ASTcall($1, NULL);
        add_loc_to_node($$, @$);
      }
    | varref
      {
        $$ = $1;
        add_loc_to_node($$, @$);
      }
    | LIT_INT
      {
        $$ = ASTint($1);
        add_loc_to_node($$, @$);
      }
    | LIT_FLOAT
      {
        $$ = ASTfloat($1);
        add_loc_to_node($$, @$);
      }
    | LIT_BOOL
      {
        $$ = ASTbool($1);
        add_loc_to_node($$, @$);
      }
    ;

arrexprs: arrexpr COMMA arrexprs
          {
            $$ = ASTarrexprs($1, $3);
            add_loc_to_node($$, @$);
          }
        | arrexpr
          {
            $$ = ASTarrexprs($1, NULL);
            add_loc_to_node($$, @$);
          }
        ;

arrexpr: expr
         {
           $$ = $1;
           add_loc_to_node($$, @$);
         }
       | BRACKET_L arrexprs BRACKET_R
         {
           $$ = $2;
           add_loc_to_node($$, @$);
         }
       ;

vardecls: vardecls vardecl
          {
            $$ = ASTvardecls($2, $1);
          }
        | vardecl
          {
            $$ = ASTvardecls($1, NULL);
          }
        ;

vardecl: basictype id SEMICOLON
         {
           $$ = ASTvardecl(ASTtype(NULL, $1), $2, NULL);
         }
       | basictype BRACKET_L exprs BRACKET_R id SEMICOLON
         {
           $$ = ASTvardecl(ASTtype($3, $1), $5, NULL);
         }
       | basictype id ASSIGN expr SEMICOLON
         {
           $$ = ASTvardecl(ASTtype(NULL, $1), $2, $4);
         }
      | basictype BRACKET_L exprs BRACKET_R id ASSIGN arrexpr SEMICOLON
         {
           $$ = ASTvardecl(ASTtype($3, $1), $5, $7);
         }
       ;

fundefs: fundef fundefs
         {
           $$ = ASTfundecls($1, $2);
         }
       | fundef
         {
           $$ = ASTfundecls($1, NULL);
         }
       ;

fundef: funheader BRACE_L BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, NULL, NULL);
        }
      | funheader BRACE_L vardecls BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(rev_vardecls($3), NULL, NULL);
        }
      | funheader BRACE_L fundefs BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, $3, NULL);
        }
      | funheader BRACE_L stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, NULL, $3);
        }
      | funheader BRACE_L vardecls fundefs BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(rev_vardecls($3), $4, NULL);
        }
      | funheader BRACE_L vardecls stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(rev_vardecls($3), NULL, $4);
        }
      | funheader BRACE_L fundefs stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, $3, $4);
        }
      | funheader BRACE_L vardecls fundefs stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(rev_vardecls($3), $4, $5);
        }
      ;

funheader: basictype id PAREN_L params PAREN_R
           {
             $$ = ASTfundecl($2, $4, $1);
           }
         | TY_VOID id PAREN_L params PAREN_R
           {
             $$ = ASTfundecl($2, $4, TY_void);
           }
         | basictype id PAREN_L PAREN_R
           {
             $$ = ASTfundecl($2, NULL, $1);
           }
         | TY_VOID id PAREN_L PAREN_R
           {
             $$ = ASTfundecl($2, NULL, TY_void);
           }
         ;

block: stmt
       {
         $$ = ASTstmts($1, NULL);
       }
     | BRACE_L stmts BRACE_R
       {
         $$ = $2;
       }
     | BRACE_L BRACE_R
       {
         $$ = NULL;
       }
     ;

varref: id BRACKET_L exprs BRACKET_R
        {
          $$ = ASTvarref($1, $3);
        }
      | id
        {
          $$ = ASTvarref($1, NULL);
        }
      ;

id: ID
    {
      $$ = ASTid($1);
      add_loc_to_node($$, @$);
    }
  ;

params: basictype id COMMA params
        {
          $$ = ASTparams(ASTparam(ASTtype(NULL, $1), $2), $4);
        }
      | basictype BRACKET_L ids BRACKET_R id COMMA params
        {
          $$ = ASTparams(ASTparam(ASTtype($3, $1), $5), $7);
        }
      | basictype id
        {
          $$ = ASTparams(ASTparam(ASTtype(NULL, $1), $2), NULL);
        }
      | basictype BRACKET_L ids BRACKET_R id
        {
          $$ = ASTparams(ASTparam(ASTtype($3, $1), $5), NULL);
        }
      ;

ids: id COMMA ids
     {
       $$ = ASTexprs(ASTvarref($1, NULL), $3);
     }
   | id
     {
       $$ = ASTexprs(ASTvarref($1, NULL), NULL);
     }
   ;

basictype: TY_BOOL
           {
             $$ = TY_bool;
           }
         | TY_INT
           {
             $$ = TY_int;
           }
         | TY_FLOAT
           {
             $$ = TY_float;
           }
         ;

%%

node_st *rev_vardecls(node_st *root) {
    node_st *curr = root;
    node_st *prev = NULL;

    while (curr) {
        node_st *next = VARDECLS_NEXT(curr);
        VARDECLS_NEXT(curr) = prev;
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
}

node_st *scanparse(node_st *root) {
    DBUG_ASSERT(root == NULL, "Started parsing with existing syntax tree.");
    yyin = fopen(globals.input_file, "r");
    if (yyin == NULL) {
        emit_message(L_ERROR, "couldn't read '%s': %s (os error %d)\n", globals.input_file, strerror(errno), errno);
    } else {
        yyparse();
    }
    abort_on_error();
    return parseresult;
}
