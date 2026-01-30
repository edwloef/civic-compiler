%{

#include <ctype.h>
#include <limits.h>

#include "ccn/ccn.h"
#include "error/error.h"
#include "globals/globals.h"
#include "palm/memory.h"
#include "palm/str.h"
#include "parser.h"

void yyerror(char const *err);
void token_action(void);

#define FILTER(token) \
    globals.col += yyleng; \
    return token;

#define YY_USER_INIT BEGIN(SAVELINE);
#define YY_USER_ACTION token_action();

%}

%option noinput
%option nounput
%option noyywrap

%x SAVELINE

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
"else"                                                   { FILTER(KW_ELSE);   }
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
                                                           yylval.cfloat = strtod(yytext, NULL);
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
                                                           BEGIN(SAVELINE);
                                                         }

" "                                                      { globals.col += 1; }
\t                                                       { globals.col += 4; }
"//".*                                                   { globals.col += yyleng; }

.                                                        { yyerror("unexpected token found, quit parsing"); }

<SAVELINE>{
  "# "[0-9]+" \"".*"\"".*\n                              {
                                                           yytext += 2;
                                                           int len = 0;
                                                           while (isdigit(yytext[len]))
                                                               len++;
                                                           yytext[len] = 0;
                                                           globals.line = atoi(yytext);
                                                           globals.line = globals.line == 0 ? 0 : globals.line - 1;
                                                           globals.col = 0;
                                                           yytext += len + 2;
                                                           len = 0;
                                                           while (yytext[len] != '"')
                                                               len++;
                                                           yytext[len] = 0;
                                                           MEMfree(globals.file);
                                                           globals.file = STRcpy(yytext);
                                                         }
  .*                                                     {
                                                           if (!globals.quiet) {
                                                               if (!globals.linemap)
                                                                   globals.linemap = HTnew_String(256);
                                                               htable_st *inner = HTlookup(globals.linemap, globals.file);
                                                               if (!inner) {
                                                                   inner = HTnew_Int(256);
                                                                   HTinsert(globals.linemap, STRcpy(globals.file), inner);
                                                               }
                                                               int *lineno = malloc(sizeof(int));
                                                               *lineno = globals.line;
                                                               HTinsert(inner, lineno, STRcpy(yytext));
                                                           }
                                                           yyless(0);
                                                           globals.col = 0;
                                                           BEGIN(INITIAL);
                                                         }
  \n                                                     { globals.line += 1; }
}

%%

void token_action(void) {
    yylloc.first_line = yylloc.last_line = globals.line;
    yylloc.first_column = globals.col;
    yylloc.last_column = globals.col + yyleng - 1;
}
