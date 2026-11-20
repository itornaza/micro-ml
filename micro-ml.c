/*******************************************************************
File:       micro-ml.c
Name:       ion-
Project:    Home Assignment 2 / MicroML compiler
Date:       7 Sep. 2007

Description: 
-  Implementation of the MicroML using the recursive descent 
algorythm. It operates as a co-routine with the lscanner.It takes as
an input a MicroML program and outputs a C file as an intermediate 
code, accompanied with a respective header "microML.h".
-  Call the program from the command line with the following options:
(assume that the compiler is build under the name "micro")
1) To get the compiled code into a C file type:

   ~$./micro <inputFile.ml> <outputFile.c>

2) To get the compiled code into the stdout type:
   
   ~$./micro <inputFile.ml>

Note: The program needs at least an input ml file to execute.

Warnings: 
1) gcc with the flag -Wall set:
lex.yy.c:1180: warning: 'yyunput' defined but not used
2) Visual Studio:
none
********************************************************************/
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "micro-ml.h"

/**********************************************
               Global variables 
***********************************************/

/* FLEX variables used also by this program */
extern int yylex();	
extern char * yytext;	
extern int yylineno;

/* Target program sources */
FILE * headerFile;   /* To store the target function prototypes */

/* Symbol table */
char functionParameters[NUMBER_OF_PARAMETERS][NUMBER_OF_LETTERS] =
                                                         {{0}, {0}};
char functionNames[NUMBER_OF_FUNCTONS][NUMBER_OF_LETTERS] = 
                                                         {{0}, {0}};

int token; 	/* current token */

/* Revision (Nov 2026): left-to-right argument evaluation.
   C leaves the order of function-argument evaluation unspecified, so
   f(READ_NUMBER(), READ_NUMBER()) could read its inputs in any order.
   Calls with two or more arguments are now emitted as a comma
   expression that stores each argument in a local temporary first:
      (_t0 = a, _t1 = b, f(_t0, _t1))
   The temporaries are locals of each generated function, so the
   scheme is safe under recursion. */
int tempCount = 0;   /* temporaries used by the current function */

/* Run a grammar routine with its C output captured in a string */
static char * capture(void (*rule)(void))
{
   char * buffer = NULL;
   size_t length = 0;
   FILE * saved = sourceFile;

   sourceFile = open_memstream(&buffer, &length);
   if (sourceFile == NULL)
   {
      perror("open_memstream");
      exit(1);
   }
   rule();
   fclose(sourceFile);
   sourceFile = saved;
   return buffer;
}/* End capture() */

/* Emit the declarations of the temporaries used by a function body */
static void declareTemps()
{
   int i;
   if (tempCount > 0)
   {
      fprintf(sourceFile, "\tint ");
      for (i = 0; i < tempCount; i++)
         fprintf(sourceFile, "%s_t%d", i ? ", " : "", i);
      fprintf(sourceFile, ";\n");
   }
}/* End declareTemps() */

/****************************
         Grammar 
*****************************/

/*********************************************************************
                              program()
        
Data in:       none
Data out:      none
Data returned: none
Task:  
Program ::= Function-definitions '?' Goal-Expression
*********************************************************************/
void program()
{
   /* Open the header file of the target program */
   headerFile = fopen("microML.h", "w");
   
   /* Initialize functionNames with the runtime library function*/
   strcpy(functionNames[0], "READ_NUMBER");
   
   /* Print the default data in the target sources */
   defaultOutput();
   
   advance();
   functionDefinitions();
   
   if (match(QUESTION_MARK))
   {
      advance();
      goalExpression();
   }/* End QUESTIONMARK */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Invalid program syntax.\n",
                     yylineno);
   }/* End else */

   /* Close the header and source file of the target program */
   fclose(headerFile);
   fclose(sourceFile);
   
}/* End program() */

/*********************************************************************
                        functionDefinitions()
          
Data in:       none
Data out:      none
Data returned: none
Task:
Function-definitions ::= 
         Function-definition Function-definitions | empty
*********************************************************************/
void functionDefinitions()
{
   if (match(FUN))
   {
      functionDefinition();
      functionDefinitions();
   }/* End FUN */

   else if (match(QUESTION_MARK))
   {
      /* empty block */
   }/* End else if */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Use FUN to declare a new function"
                     " or ? to execute.\n", yylineno);
   }/* End else */

}/* functionDefinitions() */

/*********************************************************************
                        functionDefinition()
    
Data in:       none
Data out:      none
Data returned: none
Task:
Function-definition ::= 
         FUN Function-name '(' Parameter-names ')' Expression ';'
*********************************************************************/
void functionDefinition()
{
   if (match(FUN))
   {
      advance();
      
      /* look in the symbol table to check if unique */
      if(searchItem(yytext, functionNames))
      {
         fprintf(stderr, "ERROR:ln-%d: Function already defined\n",
                        yylineno);
         exit(1);
      }/* End if */

      /* If it is unique add to the symbol table */
      addItem(yytext, functionNames);
      
      functionName();
      
      if (match(OPEN_PARENTHESIS))
      {
         fprintf(sourceFile, "%s", yytext);
         fprintf(headerFile, "%s", yytext);
         advance();
         parameterNames();
         
         if (match(CLOSE_PARENTHESIS))
         {
            fprintf(sourceFile, "%s\n{\n", yytext);
            fprintf(headerFile, "%s;\n", yytext);
            advance();
            {
               char * body;
               tempCount = 0;
               body = capture(expression);
               declareTemps();
               fprintf(sourceFile, "\treturn %s", body);
               free(body);
            }
            
            if(match(SEMICOLON))
            {
               fprintf(sourceFile, "%s\n}\n\n", yytext);
               advance();
            }/* End SEMICOLON */
         
            else
            {
               fprintf(stderr, "ERROR:ln-%d: Expecting Semicolon\n",
                              yylineno);
            }/* end else */

         }/* End CLOSE_PARENTHESIS */

         else
         {
            fprintf(stderr, "ERROR:ln-%d: Expecting closing"
                        " parenthesis\n", yylineno);
         }/* End else */
      
      }/* End OPEN_PARENTHESIS */

      else
      {
         fprintf(stderr, "ERROR:ln-%d: Expecting opening"
                        " parenthesis\n", yylineno);
      }/* End else */
   
   }/* End FUN */

}/* functionDefinition() */

/*********************************************************************
                        parameterNames()
       
Data in:       none
Data out:      none
Data returned: none
Task:
Parameter-names ::= parameter-name (',' Parameter-name )* | empty 
*********************************************************************/
void parameterNames()
{
   if (match(IDENTIFIER))
   {
      parameterName();
      
      while (match(COMMA))
      {
         fprintf(sourceFile, "%s ", yytext);
         fprintf(headerFile, "%s ", yytext);
         advance();
         parameterName();
      }/* End COMMA */
   
   }/* End IDENTIFIER */

   /* Initialize the symbol table of the function parameters to achive
      function scope */
   initializeSymbol(functionParameters);


}/* End parameterNames() */

/*********************************************************************
                        goalExpression()

Data in:       none
Data out:      none
Data returned: none
Task:
Goal-expression ::= Expression 
*********************************************************************/
void goalExpression()
{
   /* It also defines the main function of the targaet program */
   char * body;

   fprintf(sourceFile, "int main()\n{\n\tint result = 0;\n");
   tempCount = 0;
   body = capture(expression);
   declareTemps();
   fprintf(sourceFile, "\tresult = %s", body);
   free(body);
   fprintf(sourceFile, ";\n\tprintf(\"Ans: %%d\\n\", result);\n"
                     "\treturn 0;\n}\n");
   
}/* End goalExpression() */

/*********************************************************************
                           expression()
   
Data in:       none
Data out:      none
Data returned: none
Task:
Expression ::= BoolExpr1 ( 'OR' BoolExpr1 )*
*********************************************************************/
void expression()
{
   boolExpr1();
   
   while (match(LOGIC_OR))
   {
      fprintf(sourceFile, " || ");
      advance();
      boolExpr1();
   }/* End LOGIC_OR */

}/* End expression() */

/*********************************************************************
                           boolExpr1()

Data in:       none
Data out:      none
Data returned: none
Task:
BoolExpr1 ::= BoolExpr2 ( 'AND' BoolExpr2 )*
*********************************************************************/
void boolExpr1()
{
   boolExpr2();
   
   while (match(LOGIC_AND))
   {
      fprintf(sourceFile, " && ");
      advance();
      boolExpr2();
   }/* End LOGIC_OR */

}/* End boolexpr1() */

/*********************************************************************
                           boolExpr2()
    
Data in:       none
Data out:      none
Data returned: none
Task:
BoolExpr2 ::= ArithmeticExpression[
   ( '=' | '<' | '>' | '<=' | '>=' | '!=') ArithmeticExpression ] 
   | 'NOT' BoolExpr2
*********************************************************************/
void boolExpr2()
{
   if (match(LOGIC_NOT))
   {
      fprintf(sourceFile, " ! ");
      advance();
      boolExpr2();
   }/* End NOT */
   
   else  
   {
      arithmeticExpression();
      if (match(EQUAL)            || 
          match(LESS_THAN)        || 
          match(GREATER_THAN)     ||
          match(LESS_OR_EQUAL)    || 
          match(GREATER_OR_EQUAL) || 
          match(NOT_EQUAL)
          )
      {
         /* Treat separately because of the C syntax 
            difference between '=' and '=='*/
         if (match(EQUAL)) 
         {
            fprintf(sourceFile, " == ");
         }/* End if */

         else
         {
            fprintf(sourceFile, " %s ", yytext);
         }/* End else */

         advance();
         arithmeticExpression();
      }/* End relation operators */
         
   }/* end arithmeticExpression */

}/* End boolExpr2() */

/*********************************************************************
                        arithmeticExpression()
     
Data in:       none
Data out:      none
Data returned: none
Task:
ArithmeticExpression ::= Term ( ( '+' | '-' ) Term )*
*********************************************************************/
void arithmeticExpression()
{
   term();
   
   while (match(PLUS) || match(MINUS))
   {
      fprintf(sourceFile, " %s ", yytext);
      advance();
      term();
   }/* End PLUS or MINUS */

}/* End arithmeticExpression() */

/*********************************************************************
                              term()

Data in:       none
Data out:      none
Data returned: none
Task:
Term ::= Factor ( ( '*' | '/' ) Factor )*
*********************************************************************/
void term()
{
   factor();
   
   while (match(TIMES) || match(OVER))
   {
      fprintf(sourceFile, "%s", yytext);
      advance();
      factor();
   }/* End TIMES or OVER */

}/* End term() */

/*********************************************************************
                              factor()
      
Data in:       none
Data out:      none
Data returned: none
Task:
Factor ::= Number | Parameter-name-or-Function-call
         | Conditional-expression
         | ( ( '+' | '-' ) Factor )
         | READ-NUMBER
         | '(' Expression ')'
*********************************************************************/
void factor()
{
   if (match(INTEGER))
   {
      number();
   }/* End INTEGER */

   else if (match(IDENTIFIER))
   {
      parameterNameOrFunctionCall();
   }/* End IDENTIFIER */

   else if (match(IF))
   {
      conditionalExpression();
   }/* End IF */

   else if (match(PLUS) || match(MINUS))
   {
      fprintf(sourceFile, " %s ", yytext);
      advance();
      factor();
   }/* End PLUS or MINUS */

   else if(match(READ_NUMBER))
   {
      readNumber();
   }/* End READ_NUMBER */

   else if (OPEN_PARENTHESIS)
   {
      fprintf(sourceFile, "%s", yytext);
      advance();
      expression();
      if (match(CLOSE_PARENTHESIS))
      {
         fprintf(sourceFile, "%s", yytext);
         advance();
      }/* End if */

   }/* End OPEN_PARENTHESIS */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Invalid input token\n",
                     yylineno);
   }/* End ERROR handling */
   
}/* End factor() */

/*********************************************************************
                     parameterNameOrFunctionCall()

Data in:       none
Data out:      none
Data returned: none
Task:
Parameter-name-or-Function-call ::= 
         Identifier ( '(' Expression-list ')' | empty )
*********************************************************************/
void parameterNameOrFunctionCall()
{
   if (match(IDENTIFIER))
   {
      char name[NUMBER_OF_LETTERS];

      strncpy(name, yytext, NUMBER_OF_LETTERS - 1);
      name[NUMBER_OF_LETTERS - 1] = '\0';
      advance();
      
      if (match(OPEN_PARENTHESIS))
      {
         char * args[NUMBER_OF_PARAMETERS];
         int count, i, first;

         advance();
         count = expressionList(args);
         
         if (count < 2)
         {
            /* Zero or one argument: no ordering issue */
            fprintf(sourceFile, "%s(%s)", name, count ? args[0] : "");
         }
         else
         {
            /* Evaluate the arguments left to right into temporaries */
            first = tempCount;
            tempCount += count;
            fprintf(sourceFile, "(");
            for (i = 0; i < count; i++)
               fprintf(sourceFile, "_t%d = %s, ", first + i, args[i]);
            fprintf(sourceFile, "%s(", name);
            for (i = 0; i < count; i++)
               fprintf(sourceFile, "%s_t%d", i ? ", " : "", first + i);
            fprintf(sourceFile, "))");
         }
         for (i = 0; i < count; i++)
            free(args[i]);
         
         if (match(CLOSE_PARENTHESIS))
         {
            advance();
         }/* End CLOSE_PARENTHESIS */
      
         else
         {
            fprintf(stderr, "ERROR:ln-%d: Expecting closing "
                           "parenthesis\n", yylineno);
         }/* end else */

      }/* End OPEN_PARENTHESIS */

      else
      {
         /* A parameter name */
         fprintf(sourceFile, "%s", name);
      }/* End else */

   }/* End IDENTIFIER */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Invalid Identifier\n",
                     yylineno);
   }/* End else */

}/* End parameterNameOrFunctionCall() */

/*********************************************************************
                     conditionalExpression()
      
Data in:       none
Data out:      none
Data returned: none
Task:
Conditional-expression ::= 
         'IF' Expression 'THEN' Expression 'ELSE' Expression 'FI'
*********************************************************************/
void conditionalExpression()
{
   if (match(IF))
   {
      fprintf(sourceFile, "(");
      advance();
      expression();
      
      if (match(THEN))
      {
         fprintf(sourceFile, ")? (");
         advance();
         expression();
         
         if (match(ELSE))
         {
            fprintf(sourceFile, "): (");
            advance();
            expression();
            
            if (match(FI))
            {
               fprintf(sourceFile, ")");
               advance();
            }/* End FI */

            else
            {
               fprintf(stderr, "ERROR:ln-%d: Expecting FI\n",
                              yylineno);
            }/* End else */
         
         }/* End ELSE */
      
         else
         {
            fprintf(stderr, "ERROR:ln-%d: Expecting ELSE\n",
                           yylineno);
         }/* End else */

      }/* End THEN */
      
      else
      {
         fprintf(stderr, "ERROR:ln-%d: Expecting THEN\n",
                        yylineno);
      }/* End else */

   }/* End IF */

}/* End conditionalExpression() */

/*********************************************************************
                        expressionList()
         
Data in:       none
Data out:      none
Data returned: none
Task:
Expression-list ::= ( Expression ( ',' Expression )* ) | empty
*********************************************************************/
int expressionList(char * args[])
{
   int count = 0;

   if (match(INTEGER)      ||
       match(IDENTIFIER)   ||
       match(IF)           ||
       match(PLUS)         ||
       match(MINUS)        ||
       match(READ_NUMBER)
       )
   {
      args[count++] = capture(expression);
      
      while(match(COMMA))
      {
         advance();
         if (count >= NUMBER_OF_PARAMETERS)
         {
            fprintf(stderr, "ERROR:ln-%d: Too many arguments\n",
                           yylineno);
            exit(1);
         }
         args[count++] = capture(expression);
      }/* End COMMA */
   
   }/* End expression */

   return count;

}/* End expressionList() */

/*********************************************************************
                           functionName()
  
Data in:       none
Data out:      none
Data returned: none
Task:
function-name ::= Identifier
*********************************************************************/
void functionName()
{
   if (match(IDENTIFIER))
   {
      fprintf(sourceFile, "int %s", yytext);
      fprintf(headerFile, "int %s", yytext);
      advance();
   }/* End IDENTIFIER */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Invalid function name\n",
                     yylineno);
   }/* End else */

}/* End functionName() */

/*********************************************************************
                           parameterName()
      
Data in:       none
Data out:      none
Data returned: none
Task:
Parameter-name ::= Identifier
*********************************************************************/
void parameterName()
{
   if (match(IDENTIFIER))
   {
      /* Check if the parameter is unique for the given function.
         If it is append it in the symbol table */
      if(searchItem(yytext, functionParameters))
      {
         fprintf(stderr, "ERROR:ln-%d: Parameter already defined\n",
                        yylineno);
         exit(1);
      }/* End if */

      addItem(yytext, functionParameters);
   
      fprintf(sourceFile, "int %s", yytext);
      fprintf(headerFile, "int %s", yytext);
      advance();
   }/* End IDENTIFIER */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Invalid parameter name\n",
                     yylineno);
   }/* End else */

}/* End parameterName() */

/*********************************************************************
                              number()
  
Data in:       none
Data out:      none
Data returned: none
Task:
Number ::= Integer
*********************************************************************/
void number()
{
   if (match(INTEGER))
   {
      fprintf(sourceFile, "%s", yytext);
      advance();
   }/* End INTEGER */

   else
   {
      fprintf(stderr, "ERROR:ln-%d: Expecting an Integer\n",
                     yylineno);
   }/* End else */

}/* End number() */

/*********************************************************************
                              readNumber()
         
Data in:       none
Data out:      none
Data returned: none
Task:
READ-NUMBER ::= 'READ-NUMBER' '(' ')'
*********************************************************************/
void readNumber()
{
   if (match(READ_NUMBER))
   {
      fprintf(sourceFile, "%s", yytext);
      advance();
      
      if (match(OPEN_PARENTHESIS))
      {
         fprintf(sourceFile, "%s", yytext);
         advance();
         
         if (match(CLOSE_PARENTHESIS))
         {
            fprintf(sourceFile, "%s", yytext);
            advance();
         }/* End CLOSE_PARENTHESIS */

         else
         {
            fprintf(stderr, "ERROR:ln-%d: Expecting closing "
                           "parenthesis after READ_NUMBER(\n",
                           yylineno);
         }/* End else */
            
      }/* End OPEN_PARENTHESIS */

      else
      {
         fprintf(stderr, "ERROR:ln-%d: Expecting opening "
                        "parenthesis after READ_NUMBER\n", yylineno);
      }/* End else */

   }/* End READ_NUMBER */

}/* End readNumber() */

/*******************************
     Symbol Table functions 
********************************/

/*********************************************************************
                           initializeSymbol()

Task:          Initialize the symbol table with the run time library.
Data in:       symbol table.
Data out:      symbol table with changes.
Data returned: none
*********************************************************************/
void initializeSymbol(char symbolTable[][NUMBER_OF_LETTERS])
{
   int ix = 0; /* Loop counter */
   
   for (ix = 0; ix < NUMBER_OF_FUNCTONS; ++ix)
   {
      strcpy(symbolTable[ix], "");
   }/* end for */

}/* initializeSymbol() */

/*********************************************************************
                                 addItem()

Task:          Add a function in the symbol table.
Data in:       function name to be added, symbol table.
Data out:      updated functionName table.
Data returned: none
*********************************************************************/
void addItem(char * functionAdd,
             char symbolTable[][NUMBER_OF_LETTERS])
{
   int ix = 0; /* Loop counter */
   
   for (ix = 0; ix < NUMBER_OF_FUNCTONS; ++ix)
   {
      if (!strcmp(symbolTable[ix], ""))
      {
         strcpy(symbolTable[ix], functionAdd);
         break;
      }/* End if */

   }/* end for */

}/* End addParameter() */

/*********************************************************************
                              searchItem()

Task:          Find if the given function already exists in the symbol
               table.
Data in:       function name to test, symbol table
Data out:      none
Data returned: int, 1 if the function already exists, 0 otherwise.
*********************************************************************/
int searchItem(char * functionTest, 
                char symbolTable[][NUMBER_OF_LETTERS])
{
   int exists = 0;   /* flag to indicate if the functionTest exists */
   int ix = 0;       /* Loop counter */
   
   for (ix = 0; ix < NUMBER_OF_FUNCTONS; ++ix)
   {
      if (!strcmp(functionTest, symbolTable[ix]))
      {
         exists = 1;
      }/* End if */

   }/* end for */

   return exists;

}/* searchItem() */

/*******************************
      Utility Functions 
********************************/

/*********************************************************************
                                 match()

Task:          Compare the input from the scanner with the desired 
               token
Data in:       Integer value of the token to be matched.
Data out:      
Data returned: The integer value of the matched token if they match
               zero otherwise.
*********************************************************************/
int match(int t)
{
	return (token == t);
}/* End match() */

/*********************************************************************
                                 advance()

Task:          Advance the input using the scanner.
Data in:       none
Data out:      none
Data returned: none
*********************************************************************/
void advance()	/* calls yylex to get next token */
{
	token = yylex();  /* Notice that token gets an integer value
                        from the LEX retutn statements */
}/* End advance() */

/*********************************************************************
                           defaultOutput()

Task:          Print the default information in the microML files
Data in:       none
Data out:      none
Data returned: none
*********************************************************************/
void defaultOutput()	
{
   /* Initialize the files */
   fprintf(headerFile, "/* microML.h */\n\n"
                     "/* Function Prototypes */\n");
   fprintf(sourceFile, "/* MicroML program translated into C */\n\n"
                     "#include <stdio.h>\n"
                     "#include \"microML.h\"\n\n");
   
   /* Enter the runtime library prototypes to the header file */
   fprintf(headerFile,"int READ_NUMBER();\n");
            
   /* Enter the runtime library to the source file */
   fprintf(sourceFile,"int READ_NUMBER()\n"
                     "{\n"
                     "\tint x;\n"
                     "\tprintf(\"Enter Number: \");\n"
                     "\tscanf(\"%%d\", &x);\n"
                     "\treturn x;\n"
                     "}\n\n");

}/* End defaultOutput() */

/* End micro-ml.c */
