/* definitions*/
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "external_file.hpp"
#include "parser.tab.hpp"
#include "output.hpp"


%}

%option yylineno
%option noyywrap


id				([a-zA-Z][a-zA-Z0-9]*)
num				(0|[1-9][0-9]*)
string			\"([^\n\r\"\\]|\\[rnt"\\])+\"
comm			(\/\/[^\r\n]*[\r|\n|\r\n]?)
ignore			(\x20|\r|\n|\t)


%%

{ignore}			;
void				return VOID;
int					return INT;
byte				return BYTE;
bool				return BOOL;
and					return AND;
or					return OR;
not					return NOT;
true				return TRUE;
false				return FALSE;
return				return RETURN;
if					return IF;
else				return ELSE;
while				return WHILE;
break				return BREAK;
b					return B;
default				return DEFAULT;


{id}				{Ter* temp = new Ter(string(yytext),"ID"); 

yylval = static_cast<Node*>(temp);

return ID;};


{num}				{yylval =
 new Ter(string(yytext),
 "NUM"); 
 return NUM;};


{string}			{yylval = new Ter(string(yytext),
"STRING"); 
return STRING;};


{comm}				;
\:					return COLON;
\;					return SC;
,					return COMMA;
\(					return LPAREN;
\)					return RPAREN;
\[					return LBRACK;
\]					return RBRACK;
\{					return LBRACE;
\}					return RBRACE;
==					return RELOPEQ;
!=					return RELOPNEQ;
\<=					return RELOPSEQ;
>=					return RELOPLEQ;
\<					return RELOPSML;
>					return RELOPLRG;
=					return ASSIGN;
\+					return BINOPADD;
-					return BINOPSUB;
\*					return BINOPMUL;
\/					return BINOPDIV;
.					{output::errorLex(yylineno); st.set_prints(false); exit(0);}

%%

