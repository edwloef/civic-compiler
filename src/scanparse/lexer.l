%{

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "ccngen/ast.h"
#include "ccngen/enum.h"
#include "parser.h"
#include "palm/str.h"
#include "global/globals.h"
#include "palm/ctinfo.h"

extern int yyerror(char *err);

#define FILTER(token) \
  global.col += yyleng;  \
  return(token);

#define LINEBUF_SIZE 256

void token_action();

#define YY_USER_ACTION token_action();

%}

%option noyywrap
%option nounput
%option yylineno

%x comment

%%

"("                                                      { FILTER(PAREN_L);   }
")"                                                      { FILTER(PAREN_R);   }
"["                                                      { FILTER(BRACKET_L); }
"]"                                                      { FILTER(BRACKET_R); }
"{"                                                      { FILTER(BRACE_L);   }
"}"                                                      { FILTER(BRACE_R);   }

","                                                      { FILTER(COMMA);     }
";"                                                      { FILTER(SEMICOLON); }

"!"                                                      { FILTER(BANG);    }
"+"                                                      { FILTER(PLUS);    }
"-"                                                      { FILTER(MINUS);   }
"*"                                                      { FILTER(STAR);    }
"/"                                                      { FILTER(SLASH);   }
"%"                                                      { FILTER(PERCENT); }
"<="                                                     { FILTER(LE);      }
"<"                                                      { FILTER(LT);      }
">="                                                     { FILTER(GE);      }
">"                                                      { FILTER(GT);      }
"=="                                                     { FILTER(EQ);      }
"!="                                                     { FILTER(NE);      }
"&&"                                                     { FILTER(AND);     }
"||"                                                     { FILTER(OR);      }
"="                                                      { FILTER(ASSIGN);  }

"bool"                                                   { FILTER(TY_BOOL);  }
"int"                                                    { FILTER(TY_INT);   }
"float"                                                  { FILTER(TY_FLOAT); }
"void"                                                   { FILTER(TY_VOID);  }

"extern"                                                 { FILTER(KW_EXTERN); }
"export"                                                 { FILTER(KW_EXPORT); }
"if"                                                     { FILTER(KW_IF);     }
"else"                                                   { FILTER(KW_ELSE);     }
"while"                                                  { FILTER(KW_WHILE);  }
"do"                                                     { FILTER(KW_DO);     }
"for"                                                    { FILTER(KW_FOR);    }
"return"                                                 { FILTER(KW_RETURN); }

"true"                                                   {
                                                           yylval.cbool = true;
                                                           FILTER(LIT_BOOL);
                                                         }
"false"                                                  {
                                                           yylval.cbool = false;
                                                           FILTER(LIT_BOOL);
                                                         }

[0-9]+                                                   {
                                                           yylval.cint = atoi(yytext);
                                                           FILTER(LIT_INT);
                                                         }

(([0-9]+(\.[0-9]*)?)|([0-9]*\.[0-9]+))([Ee][+-]?[0-9]+)? {
                                                           yylval.cfloat = atof(yytext);
                                                           FILTER(LIT_FLOAT);
                                                         }

[A-Za-z][A-Za-z0-9_]*                                    {
                                                           yylval.cstr = STRcpy(yytext);
                                                           FILTER(ID);
                                                         }

\r?\n.*                                                  {
                                                           global.line += 1;
                                                           global.col = 0;
                                                           yyless(1);
                                                         }

[ \t]                                                    { global.col += yyleng; }

"//".*                                                   { global.col += yyleng; }
"/*"                                                     {
                                                           global.col += yyleng;
                                                           BEGIN(comment);
                                                         }
<comment>([^*]|\*[^/])*                                  { global.col += yyleng; }
<comment>"*/"                                            {
                                                           global.col += yyleng;
                                                           BEGIN(INITIAL);
                                                         }
<comment>\r?\n.*                                         {
                                                           global.line += 1;
                                                           global.col = 0;
                                                           yyless(1);
                                                         }

%%

void token_action() {
    yylloc.first_line = yylloc.last_line = global.line;
    yylloc.first_column = global.col;
    yylloc.last_column = global.col + yyleng - 1;
}
