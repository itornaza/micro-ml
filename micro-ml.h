/*********************************************************************
File:       micro-ml.h
Name:       ion-
Project:    Home Assignment 2 / MicroML compiler
Date:       7 Sep. 2007

Description: 
- Header File for MicroML compiler. Contains the global constants 
and the function definitions. Its purpose is to help the communication
between the scanner and the parser. For more information about the
functions consult the function headers in the micro-ml.c file
*********************************************************************/

/*************************************
         Global Constants   
**************************************/

/* The C file of the target program is opened from the scanner
   and further processed by the parser */
extern FILE * sourceFile;   /* defined in micro-ml.l */

/* FLEX - parser internal communication */
enum {INTEGER = 400,
      PLUS,
      MINUS,
      TIMES,
      OVER,
      OPEN_PARENTHESIS,
      CLOSE_PARENTHESIS,
      SEMICOLON,
      COMMA,
      QUESTION_MARK,
      EQUAL,
      LESS_THAN,
      GREATER_THAN,
      LESS_OR_EQUAL,
      GREATER_OR_EQUAL,
      NOT_EQUAL,
      LOGIC_OR,
      LOGIC_AND,
      LOGIC_NOT,
      IF,
      THEN,
      ELSE,
      FI,
      READ_NUMBER,
      FUN,
      IDENTIFIER
      };

/* Constants used only by the parser */
#define NUMBER_OF_PARAMETERS 100
#define NUMBER_OF_FUNCTONS 100
#define NUMBER_OF_LETTERS 50

/*************************************
         Function Prototypes
**************************************/

/* Grammar */
void program();
void functionDefinitions();
void functionDefinition();
void parameterNames();
void goalExpression();
void expression();
void boolExpr1();
void boolExpr2();
void arithmeticExpression();
void term();
void factor();
void parameterNameOrFunctionCall();
void conditionalExpression();
int expressionList(char * args[]);
void functionName();
void parameterName();
void number();
void readNumber();

/* Symbol table functions */
void initializeSymbol(char symbolTable[][NUMBER_OF_LETTERS]);
void addItem( char *, char symbolTable[][NUMBER_OF_LETTERS] );
int searchItem( char *, char symbolTable[][NUMBER_OF_LETTERS] );

/* Utility Functions */
int match(int t);
void advance();
void defaultOutput();

/* End micro-ml.h */
