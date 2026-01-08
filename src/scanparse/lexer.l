%{

#include <limits.h>

#include "ccn/ccn.h"
#include "globals/globals.h"
#include "palm/str.h"
#include "parser.h"

int yyerror(char *err);

#define FILTER(token) \
    globals.col += yyleng; \
    return(token);

%}

%option noyywrap
%option nounput
%option noinput

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

(0[xX])?[0-9]+                                           {
                                                           errno = 0;
                                                           long val = strtol(yytext, NULL, 0);
                                                           if (errno == ERANGE || val < INT_MIN || val > INT_MAX)
                                                               yyerror("integer literal out of range");
                                                           yylval.cint = val;
                                                           FILTER(LIT_INT);
                                                         }

(([0-9]+(\.[0-9]*)?)|([0-9]*\.[0-9]+))([Ee][+-]?[0-9]+)? {
                                                           errno = 0;
                                                           yylval.cfloat = strtof(yytext, NULL);
                                                           if (errno == ERANGE)
                                                               yyerror("floating point literal out of range");
                                                           FILTER(LIT_FLOAT);
                                                         }

[A-Za-z][A-Za-z0-9_]*                                    {
                                                           yylval.cstr = STRcpy(yytext);
                                                           FILTER(ID);
                                                         }

\n                                                       {
                                                           globals.line += 1;
                                                           globals.col = 0;
                                                         }

[ \t]                                                    { globals.col += yyleng; }

"//".*                                                   { globals.col += yyleng; }
"/*"                                                     {
                                                           globals.col += yyleng;
                                                           BEGIN(comment);
                                                         }
<comment>{
  ([^*\n]*)|("*"+[^*/\n]*)                               { globals.col += yyleng; }
  "*/"                                                   {
                                                           globals.col += yyleng;
                                                           BEGIN(INITIAL);
                                                         }
  \n                                                     {
                                                           globals.line += 1;
                                                           globals.col = 0;
                                                         }
}

%%
