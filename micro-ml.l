%{
/*******************************************************************
File:       micro-ml.l
Name:       ion-
Project:    Home Assignment 2 / MicroML compiler
Date:       7 Sep. 2007
   
Description: 
-  Scanner for the MicroML compiler. It operates as a co-routine
with the parser program.
-  Notice that main is defined here!
-  Uses the microML.h file to read the values of the tokens that are
also used by the parser.
   
Warnings: 
********************************************************************/
#include <stdio.h>
#include "micro-ml.h"
   
FILE * sourceFile;   /* target C file, shared with the parser */
extern void program();
%}

DIGIT    [0-9]
ID       [a-zA-Z_][a-zA-Z_0-9]*
%x COMMENT

%option noyywrap nounput noinput

%%
{DIGIT}+       { return INTEGER; }
"+"            { return PLUS; }
"-"            { return MINUS; }
"*"            { return TIMES; }
"/"            { return OVER; }
"("            { return OPEN_PARENTHESIS; }
")"            { return CLOSE_PARENTHESIS; }
";"            { return SEMICOLON; }
","            { return COMMA; }
"?"            { return QUESTION_MARK; }
"="            { return EQUAL; }
"<"            { return LESS_THAN; }
">"            { return GREATER_THAN; }
"<="           { return LESS_OR_EQUAL; }
">="           { return GREATER_OR_EQUAL; }
"!="           { return NOT_EQUAL; }
"OR"           { return LOGIC_OR; }
"AND"          { return LOGIC_AND; }
"NOT"          { return LOGIC_NOT; }
"IF"           { return IF; }
"ELSE"         { return ELSE; }
"THEN"         { return THEN; }
"FI"           { return FI; }
"READ_NUMBER"  { return READ_NUMBER; }
"FUN"          { return FUN; }

"(*"             { BEGIN COMMENT; } 
<COMMENT><<EOF>> { printf("!!!Error: End of file inside a comment\n");
                   exit(1); }
<COMMENT>.|\n    { /* Eat up everything inside a comment */; }
<COMMENT>"*)"    { BEGIN INITIAL; }

[ \t]+         /* Eat up whitespace */;
"\n"           { yylineno++; }
{ID}           {  return IDENTIFIER; }
<<EOF>>        { return EOF; }
.        { fprintf(stderr, "Unrecognized character: %s\n", yytext ); }

%%

int main(int argc, char *argv[])
{
   /* If both the arguments are present */
	if (argc > 2)
   {
	  yyin = fopen(argv[1], "r");
      sourceFile = fopen(argv[2], "w");
      program();	/* Call parser program */
   }/* End if */

   /* Else if only the first argument is present */
   else if (argc > 1)
   {
      yyin = fopen(argv[1], "r");
      sourceFile = stdout;
      program();	/* Call parser program */
   }/* else if */
   
   /* Else if run without arguments */
	else
   {
		fprintf(stderr, "ERROR: please provide an input ml file\n\n");
   }/* End else */
		
   return 0;	
}
