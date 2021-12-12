%{
#include "JAST/jast_api.h"

#include <cassert>
#include <errno.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern int32_t line_num;  /* declared in scanner.l */
extern char buffer[];     /* declared in scanner.l */
extern FILE *yyin;        /* declared by lex */
extern char *yytext;      /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}


%code requires {
    struct AstNode;
}

    /* For yylval */
%union {
    char *identifier;
    int integer;
    double real;
    bool boolean;
    AstNode *node;
};

%type <identifier> ID INTEGER REAL STRING BOOLEAN STRING_LITERAL
%type <identifier> ASSIGN
%type <integer> INT_LITERAL
%type <real> REAL_LITERAL
%type <boolean> TRUE FALSE NegOrNot

%type <identifier> ProgramName ScalarType FunctionName 

%type <node> IdList Type ArrType ArrDecl ReturnType 
%type <node> IntegerAndReal StringAndBoolean LiteralConstant
%type <node> VariableReference FunctionInvocation
%type <node> Expression Expressions ExpressionList
%type <node> Simple Condition While For Return FunctionCall
%type <node> ArrRefs ArrRefList
%type <node> Statement Statements StatementList CompoundStatement
%type <node> Declaration Declarations DeclarationList
%type <node> Function Functions FunctionList 
%type <node> FunctionDeclaration FunctionDefinition
%type <node> FormalArg FormalArgs FormalArgList
%type <node> ElseOrNot


    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

 /* Locations */
%locations

%%

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /*  End of ProgramBody */
    END {
        root = NewProgramNode(@1.first_line, @1.first_column, $1, $3, $4, $5);
        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon { $$ = NULL; }
    |
    Declarations
;

Declarations:
    Declaration 
    |
    Declarations Declaration { $$ = AddSiblingNode($1, $2); }
;

FunctionList:
    Epsilon { $$ = NULL; }
    |
    Functions 
;

Functions:
    Function
    |
    Functions Function { $$ = AddSiblingNode($1, $2); }
;

Function:
    FunctionDeclaration
    |
    FunctionDefinition
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON
    { $$ = NewFunctionNode(@1.first_line, @1.first_column, $1, $3, $5, NULL);}
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END
    { $$ = NewFunctionNode(@1.first_line, @1.first_column, $1, $3, $5, $6);}
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon { $$ = NULL; }
    |
    FormalArgs
;

FormalArgs:
    FormalArg
    |
    FormalArgs SEMICOLON FormalArg { $$ = AddSiblingNode($1, $3); }
;

FormalArg:
    IdList COLON Type { $$ = NewDeclarationNode_Type(@1.first_line, @1.first_column, $1, $3, "parameter"); }
;

IdList:
    ID { $$ = NewIdNode(@1.first_line, @1.first_column, $1); }
    |
    IdList COMMA ID { $$ = AddSiblingNode($1, NewIdNode(@3.first_line, @3.first_column, $3)); }
;

ReturnType:
    COLON ScalarType { $$ = NewScalerTypeNode(@2.first_line, @2.first_column, $2); }
    |
    Epsilon {$$ = NULL; }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON { $$ = NewDeclarationNode_Type(@1.first_line, @1.first_column, $2, $4, "variable"); }
    |
    VAR IdList COLON LiteralConstant SEMICOLON  { $$ = NewDeclarationNode_LiteralConstant(@1.first_line, @1.first_column, $2, $4); }
;

Type:
    ScalarType { $$ = NewScalerTypeNode(@1.first_line, @1.first_column, $1); }
    |
    ArrType
;

ScalarType:
    INTEGER 
    |
    REAL 
    |
    STRING
    |
    BOOLEAN 
;

ArrType:
    ArrDecl ScalarType { $$ = NewArrTypeNode(@1.first_line, @1.first_column, $1, $2); }
;

ArrDecl:
    ARRAY INT_LITERAL OF { $$ = NewIntValueNode(@1.first_line, @1.first_column, $2); }
    |
    ArrDecl ARRAY INT_LITERAL OF { $$ = AddSiblingNode($1, NewIntValueNode(@1.first_line, @1.first_column, $3)); }
;

LiteralConstant:
    NegOrNot INT_LITERAL { $$ = NewLiteralIntNode(@2.first_line, $1 ? @1.first_column : @2.first_column, $1 ? -$2 : $2); }
    |
    NegOrNot REAL_LITERAL { $$ = NewLiteralRealNode(@2.first_line, $1 ? @1.first_column : @2.first_column, $1 ? -$2 : $2); }
    |
    StringAndBoolean
;

NegOrNot:
    Epsilon { $$ = false; }
    |
    MINUS %prec UNARY_MINUS { $$ = true; }
;

StringAndBoolean:
    STRING_LITERAL { $$ = NewLiteralStringNode(@1.first_line, @1.first_column, $1); }
    |
    TRUE { $$ = NewLiteralBooleanNode(@1.first_line, @1.first_column, $1); }
    |
    FALSE { $$ = NewLiteralBooleanNode(@1.first_line, @1.first_column, $1); }
;

IntegerAndReal:
    INT_LITERAL { $$ = NewLiteralIntNode(@1.first_line, @1.first_column, $1); }
    |
    REAL_LITERAL { $$ = NewLiteralRealNode(@1.first_line, @1.first_column, $1); }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement
    |
    Simple
    |
    Condition
    |
    While
    |
    For
    |
    Return
    |
    FunctionCall
;

CompoundStatement:
    BEGIN_
    DeclarationList
    StatementList
    END
    { $$ = NewCompoundStatementNode(@1.first_line, @1.first_column, $2, $3); }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON { $$ = NewAssignNode(@2.first_line, @2.first_column, $1, $3); }
    |
    PRINT Expression SEMICOLON { $$ = NewPrintNode(@1.first_line, @1.first_column, $2); }
    |
    READ VariableReference SEMICOLON { $$ = NewReadNode(@1.first_line, @1.first_column, $2); }
;

VariableReference:
    ID ArrRefList { $$ = NewVariableRefNode(@1.first_line, @1.first_column, $1, $2); }
;

ArrRefList:
    Epsilon { $$ = NULL; }
    |
    ArrRefs
;

ArrRefs:
    L_BRACKET Expression R_BRACKET { $$ = $2; }
    |
    ArrRefs L_BRACKET Expression R_BRACKET { $$ = AddSiblingNode($1, $3); }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF
    { $$ = NewConditionNode(@1.first_line, @1.first_column, $2, $4, $5); }
;

ElseOrNot:
    ELSE
    CompoundStatement { $$ = $2; }
    |
    Epsilon { $$ = NULL; }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO
    { $$ = NewWhileNode(@1.first_line, @1.first_column, $2, $4); }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO
    { $$ = NewForNode(@1.first_line, @1.first_column, 
                      NewIdNode(@2.first_line, @2.first_column, $2), 
                      NewIdNode(@3.first_line, @3.first_column, $3), 
                      NewIntValueNode(@4.first_line, @4.first_column, $4),
                      NewIntValueNode(@6.first_line, @6.first_column, $6),
                      $8); }
;

Return:
    RETURN Expression SEMICOLON
    { $$ = NewReturnNode(@1.first_line, @1.first_column, $2);}
;

FunctionCall:
    FunctionInvocation SEMICOLON
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS { $$ = NewFunctionInvocationNode(@1.first_line, @1.first_column, $1, $3); }
;

ExpressionList:
    Epsilon { $$ = NULL; }
    |
    Expressions
;

Expressions:
    Expression
    |
    Expressions COMMA Expression { $$ = AddSiblingNode($1, $3); }
;

StatementList:
    Epsilon { $$ = NULL; }
    |
    Statements
;

Statements:
    Statement
    |
    Statements Statement { $$ = AddSiblingNode($1, $2); }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS { $$ = $2; }
    |
    MINUS Expression %prec UNARY_MINUS { $$ = NewExpressionNode(@1.first_line, @1.first_column, "neg", NULL, $2); }
    |
    Expression MULTIPLY Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "*", $1, $3); }
    |
    Expression DIVIDE Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "/", $1, $3); }
    |
    Expression MOD Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "mod", $1, $3); }
    |
    Expression PLUS Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "+", $1, $3); }
    |
    Expression MINUS Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "-", $1, $3); }
    |
    Expression LESS Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "<", $1, $3); }
    |
    Expression LESS_OR_EQUAL Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "<=", $1, $3); }
    |
    Expression GREATER Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, ">", $1, $3); }
    |
    Expression GREATER_OR_EQUAL Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, ">=", $1, $3); }
    |
    Expression EQUAL Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "=", $1, $3); }
    |
    Expression NOT_EQUAL Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "<>", $1, $3); }
    |
    NOT Expression { $$ = NewExpressionNode(@1.first_line, @1.first_column, "not", NULL, $2); }
    |
    Expression AND Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "and", $1, $3); }
    |
    Expression OR Expression { $$ = NewExpressionNode(@2.first_line, @2.first_column, "or", $1, $3); }
    |
    IntegerAndReal { $$ = NewExpressionNode(@1.first_line, @1.first_column, "constant", $1, NULL); }
    |
    StringAndBoolean { $$ = NewExpressionNode(@1.first_line, @1.first_column, "constant", $1, NULL); }
    |
    VariableReference { $$ = NewExpressionNode(@1.first_line, @1.first_column, "VariableReference", $1, NULL); }
    |
    FunctionInvocation { $$ = NewExpressionNode(@1.first_line, @1.first_column, "FunctionInvocation", $1, NULL); }
;

    /*
       misc
            */
Epsilon:
;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, buffer, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./parser <filename> [--dump-ast]\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed:");
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        //PrintAstNode(root, 0); //DBG : print for hw4 developing
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    SymTab_Init();
    VisitAstNode(root);

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
