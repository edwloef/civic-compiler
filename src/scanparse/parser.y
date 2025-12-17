%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "palm/memory.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "palm/str.h"
#include "ccngen/ast.h"
#include "ccngen/enum.h"
#include "global/globals.h"

static node_st *parseresult = NULL;
extern int yylex();
int yyerror(char *errname);
extern FILE *yyin;

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

%token PAREN_L PAREN_R BRACKET_L BRACKET_R BRACE_L BRACE_R
%token COMMA SEMICOLON
%token BANG PLUS MINUS STAR SLASH PERCENT LE LT GE GT EQ NE AND OR ASSIGN
%token TY_INT TY_FLOAT TY_BOOL TY_VOID
%token KW_EXTERN KW_EXPORT KW_IF KW_ELSE KW_WHILE KW_DO KW_FOR KW_RETURN

%token <cbool> LIT_BOOL
%token <cint> LIT_INT
%token <cfloat> LIT_FLOAT
%token <cstr> ID

%type <node> decls stmts exprs arrexprs ids params
%type <node> program decl stmt expr arrexpr block varref id
%type <node> vardecl vardecls fundef fundefs funheader
%type <basic_type> basictype

%nonassoc "none"
%nonassoc KW_ELSE
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left STAR SLASH PERCENT
%right "monop"

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
      }
    | KW_EXTERN basictype BRACKET_L exprs BRACKET_R id SEMICOLON
      {
        $$ = ASTvardecl(ASTtype($4, $2), $6, NULL);
        VARDECL_EXTERNAL($$) = true;
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
      }
    | fundef
      {
        $$ = $1;
      }
    | vardecl
      {
        $$ = $1;
      }
    ;

stmts: stmt stmts
       {
         $$ = ASTstmts($1, $2);
       }
     | stmt
       {
         $$ = ASTstmts($1, NULL);
       }
     ;

stmt: varref ASSIGN arrexpr SEMICOLON
      {
        $$ = ASTassign($1, $3);
      }
    | id PAREN_L exprs PAREN_R SEMICOLON
      {
        $$ = ASTcall($1, $3);
      }
    | id PAREN_L PAREN_R SEMICOLON
      {
        $$ = ASTcall($1, NULL);
      }
    | KW_IF PAREN_L expr PAREN_R block %prec "none"
      {
        $$ = ASTifelse($3, $5, NULL);
      }
    | KW_IF PAREN_L expr PAREN_R block KW_ELSE block
      {
        $$ = ASTifelse($3, $5, $7);
      }
    | KW_WHILE PAREN_L expr PAREN_R block
      {
        $$ = ASTwhile($3, $5);
      }
    | KW_DO block KW_WHILE PAREN_L expr PAREN_R SEMICOLON
      {
        $$ = ASTdowhile($2, $5);
      }
    | KW_FOR PAREN_L TY_INT id ASSIGN expr COMMA expr PAREN_R block
      {
        $$ = ASTfor($4, $6, $8, ASTint(1), $10);
      }
    | KW_FOR PAREN_L TY_INT id ASSIGN expr COMMA expr COMMA expr PAREN_R block
      {
        $$ = ASTfor($4, $6, $8, $10, $12);
      }
    | KW_RETURN SEMICOLON
      {
        $$ = ASTreturn(NULL);
      }
    | KW_RETURN expr SEMICOLON
      {
        $$ = ASTreturn($2);
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
      }
    | PLUS expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_pos);
      }
    | MINUS expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_neg);
      }
    | BANG expr %prec "monop"
      {
        $$ = ASTmonop($2, MO_not);
      }
    | expr STAR expr
      {
        $$ = ASTbinop($1, $3, BO_mul);
      }
    | expr SLASH expr
      {
        $$ = ASTbinop($1, $3, BO_div);
      }
    | expr PERCENT expr
      {
        $$ = ASTbinop($1, $3, BO_mod);
      }
    | expr PLUS expr
      {
        $$ = ASTbinop($1, $3, BO_add);
      }
    | expr MINUS expr
      {
        $$ = ASTbinop($1, $3, BO_sub);
      }
    | expr LT expr
      {
        $$ = ASTbinop($1, $3, BO_lt);
      }
    | expr LE expr
      {
        $$ = ASTbinop($1, $3, BO_le);
      }
    | expr GT expr
      {
        $$ = ASTbinop($1, $3, BO_gt);
      }
    | expr GE expr
      {
        $$ = ASTbinop($1, $3, BO_ge);
      }
    | expr EQ expr
      {
        $$ = ASTbinop($1, $3, BO_eq);
      }
    | expr NE expr
      {
        $$ = ASTbinop($1, $3, BO_ne);
      }
    | expr AND expr
      {
        $$ = ASTbinop($1, $3, BO_and);
      }
    | expr OR expr
      {
        $$ = ASTbinop($1, $3, BO_or);
      }
    | PAREN_L expr PAREN_R
      {
        $$ = $2;
      }
    | id PAREN_L exprs PAREN_R
      {
        $$ = ASTcall($1, $3);
      }
    | id PAREN_L PAREN_R
      {
        $$ = ASTcall($1, NULL);
      }
    | varref
      {
        $$ = $1;
      }
    | LIT_INT
      {
        $$ = ASTint($1);
      }
    | LIT_FLOAT
      {
        $$ = ASTfloat($1);
      }
    | LIT_BOOL
      {
        $$ = ASTbool($1);
      }
    ;

arrexprs: arrexpr COMMA arrexprs
          {
            $$ = ASTarrexprs($1, $3);
          }
        | arrexpr
          {
            $$ = ASTarrexprs($1, NULL);
          }
        ;

arrexpr: expr
         {
           $$ = $1;
         }
       | BRACKET_L arrexprs BRACKET_R
         {
           $$ = $2;
         }
       ;

vardecls: vardecl vardecls
          {
            $$ = ASTvardecls($1, $2);
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
          FUNDECL_BODY($$) = ASTfunbody($3, NULL, NULL);
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
          FUNDECL_BODY($$) = ASTfunbody($3, $4, NULL);
        }
      | funheader BRACE_L vardecls stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, NULL, $4);
        }
      | funheader BRACE_L fundefs stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody(NULL, $3, $4);
        }
      | funheader BRACE_L vardecls fundefs stmts BRACE_R
        {
          $$ = $1;
          FUNDECL_BODY($$) = ASTfunbody($3, $4, $5);
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
    }
  ;

params: basictype id COMMA params
        {
          $$ = ASTparams(ASTparam($2, NULL, $1), $4);
        }
      | basictype BRACKET_L ids BRACKET_R id COMMA params
        {
          $$ = ASTparams(ASTparam($5, $3, $1), $7);
        }
      | basictype id
        {
          $$ = ASTparams(ASTparam($2, NULL, $1), NULL);
        }
      | basictype BRACKET_L ids BRACKET_R id
        {
          $$ = ASTparams(ASTparam($5, $3, $1), NULL);
        }
      ;

ids: id COMMA ids
     {
       $$ = ASTids($1, $3);
     }
   | id
     {
       $$ = ASTids($1, NULL);
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

int yyerror(char *error)
{
    CTI(CTI_ERROR, true, "%s:%d:%d: %s\n", global.input_file, global.line + 1,
        global.col + 1, error);
    CTIabortOnError();
    return 0;
}

node_st *scanparse(node_st *root)
{
    DBUG_ASSERT(root == NULL, "Started parsing with existing syntax tree.");
    yyin = fopen(global.input_file, "r");
    if (yyin == NULL) {
        CTI(CTI_ERROR, true, "couldn't read '%s'", global.input_file);
        CTIabortOnError();
    }
    yyparse();
    return parseresult;
}
