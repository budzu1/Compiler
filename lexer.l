%{

#define YY_NO_UNPUT

#include "parser.tab.hh"
#include <cstdint>
#include <cstdio>


uint64_t lines = 1;

static int lex(int token);
static int lex_num(int token);
static int lex_var(int token);

%}

WHITES  [ \t]

%%

\#.*\n              {   ++lines;                        }

\n                  {   ++lines;                        }

\;                  {   return lex(YY_SEMICOLON);       }
\,                  {   return lex(YY_COMMA);           }

\+                  {   return lex(YY_ADD);             }
\-                  {   return lex(YY_SUB);             }
\*                  {   return lex(YY_MUL);             }
\/                  {   return lex(YY_DIV);             }
\%                  {   return lex(YY_MOD);             }

\:\=                {   return lex(YY_ASSIGN);          }

\=                  {   return lex(YY_EQ);              }
\!\=                {   return lex(YY_NEQ);             }
\<                  {   return lex(YY_LT);              }
\>                  {   return lex(YY_GT);              }
\<\=                {   return lex(YY_LEQT);            }
\>\=                {   return lex(YY_GEQT);            }

"PROGRAM"           {   return lex(YY_PROGRAM);         }
"IS"                {   return lex(YY_IS);              }
"END"               {   return lex(YY_END);             }
"PROCEDURE"         {   return lex(YY_PROCEDURE);       }
"BEGIN"             {   return lex(YY_BEGIN);           }

"IF"                {   return lex(YY_IF);              }
"THEN"              {   return lex(YY_THEN);            }
"ELSE"              {   return lex(YY_ELSE);            }
"ENDIF"             {   return lex(YY_ENDIF);           }

"WHILE"             {   return lex(YY_WHILE);           }
"DO"                {   return lex(YY_DO);              }
"ENDWHILE"          {   return lex(YY_ENDWHILE);        }
"REPEAT"            {   return lex(YY_REPEAT);          }
"UNTIL"             {   return lex(YY_UNTIL);           }
"FOR"               {   return lex(YY_FOR);             }
"FROM"              {   return lex(YY_FROM);            }
"TO"                {   return lex(YY_TO);              }
"DOWNTO"            {   return lex(YY_DOWNTO);          }
"ENDFOR"            {   return lex(YY_ENDFOR);          }

"READ"              {   return lex(YY_READ);            }
"WRITE"             {   return lex(YY_WRITE);           }

"T"                 {   return lex(YY_T);               }

\(                  {   return lex(YY_L_BRACKET_R);     }
\)                  {   return lex(YY_R_BRACKET_R);     }

\[                  {   return lex(YY_L_BRACKET_S);     }
\]                  {   return lex(YY_R_BRACKET_S);     }

\:                  {   return lex(YY_ARR_RANGE);       }

[_a-z]+             {   return lex_var(YY_VARIABLE);    }

[0-9]+              {   return lex_num(YY_NUM);         }


%%

static int lex(int token)
{
    yylloc.first_line = lines;
    yylloc.last_line += lines;
    yylval.ptoken.line = lines;
    yylval.ptoken.value = 0;
    yylval.ptoken.str = nullptr;

    return token;
}

static int lex_num(int token)
{
    yylval.ptoken.line = lines;
    yylval.ptoken.value = strtoull(yytext, nullptr, 10);
    yylval.ptoken.str = nullptr;

    return token;
}

static int lex_var(int token)
{
    yylval.ptoken.value = 0;
    yylval.ptoken.line = lines;
    yylval.ptoken.str = new std::string(yytext);

    return token;
}

int yywrap()
{
    return 1;
}