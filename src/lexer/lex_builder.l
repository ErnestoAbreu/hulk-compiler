%{
#include <stdio.h>
%}

NUMBER    [0-9][0-9]*
ID       [a-zA-Z][a-zA-Z0-9]*

%%

{ID}                        printf("Identifier\n");
{NUMBER}                    printf("Number\n");
{NUMBER}"."{NUMBER}         printf("Number\n");

"+"                         printf("Plus\n");
"-"                         printf("Minus\n");
"*"                         printf("Times\n");
"/"                         printf("Divide\n");
"^"                         printf("Power\n");

";"                         printf("Semicolon\n");
"("                         printf("Open parentheses\n");
")"                         printf("Close parentheses\n");
"{"                         printf("Open bracket\n");
"}"                         printf("Close bracket\n");
[" \t\n"]
%%