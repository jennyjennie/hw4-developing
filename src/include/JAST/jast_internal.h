
#ifndef __JAST_INTERNAL_H__
#define __JAST_INTERNAL_H__

#include "jast.h"

// -----------------------------------------------------------------
// Definition struct and constants for symbol table.
// -----------------------------------------------------------------
typedef enum SymbolValue { 
	kUnknown = -1, kProgram = 0, kFunction, kParameter, kVariable, kLoopVar, kConstant, kInteger, kReal, kBoolean, kString, kVoid,
	kADD, kMINUS, kMULTIPLY, kDIVIDE, kNEG, kMOD, kAND, kOR, kNOT, kLT, kLE, kEQ, kGE, kGT, kNE, kSTRCAT
} SymbolValue_t;

typedef struct Symbol {
	int nLevel;
	char *pszName;
	char *pszKind;
	char *pszScalerType;
	char *pszTypeStr;
	char *pszAttr;
	SymbolValue_t nSymKind;
	SymbolValue_t nSymType;
	AstNode *pAst;			// FunctionNode if nSymKind == kFunction, else TypeNode, or ProgramNode for kProgram.
} Symbol_t;

// -----------------------------------------------------------------
// Definition struct for each non-terminal content body.
// -----------------------------------------------------------------
struct IdNode {
	const char *pszName;
};

struct IntValueNode {
	int nValue; 
};

struct ProgramNode {
	const char *pszName;
	AstNode *pFirstDeclarationNode; // a link list of DeclarationNode.
	AstNode *pFirstFunctionNode; 	// a link list of FunctionNode.
	AstNode *pCompoundStatementNode;
};

struct DeclarationNode {
	const char *pszKind;			// string of "variable", "parameter", "loop_var", or "constant".
	SymbolValue_t nKind;			// symbol value of declaration kind: kVariable, kParameter, kLoopVar, or kConstant.
	AstNode *pFirstIdNode; 			// a link list of IdNode for variables.
	AstNode *pTypeNode;
	AstNode *pLiteralNode;			// constant declaration, NULL if it was not created at Declaration->LiteralConstant.
};

struct TypeNode {
	const char *pszTypeStr;
	const char *pszScalerType;
	SymbolValue_t nScalerType;
	AstNode *pFirstIntNode;  		// a link list of IntNode for array dimensions.
};

struct LiteralNode {
	const char *pszType;			// string of "integer", "real", "string", or "boolean".
	SymbolValue_t nType;			// symbol value of literal type: kInteger, kReal, kString, or kBoolean.
	int nLiteralInt;				// literal value for pszType == "integer"
	double dLiteralReal;			// literal value for pszType == "real"
	bool nLiteralBoolean;			// literal value for pszType == "boolean"
	const char *pszLiteralString;	// literal value for pszType == "string"
	const char *pszStr;				// string ptr pointing to the literal content for print.
};

struct ExpressionNode {
	const char *pszOp;				// string of math operator, or indicating special operand with "constant", "VariableReference", or "FunctionInvocation".
	AstNode *pLeftNode;	
	AstNode *pRightNode;
	SymbolValue_t nOp;				// symbol value for the operator. 
	SymbolValue_t nResultType;		// filled by visit(), the resultant type of this expression after calculation.
};

struct CompoundStatementNode {
	AstNode *pFirstDeclarationNode; // a link list of DeclarationNode.
	AstNode *pFirstStatementNode; 	// a link list of StatementNode.
};

struct PrintNode {
	AstNode *pExpressionNode;
};

struct FunctionInvocationNode {
	const char *pszFuncName;
	AstNode *pFirstExpressionNode; 	// a link list of ExpressionNode.
	SymbolValue_t nReturnType;		// filled by visit(), symbol value of the return type.
};

struct FunctionNode {
	const char *pszFuncName;
	const char *pszReturnType;		// if pReturnTypeNode == NULL, pszReturnType is "void".
	const char *pszParamTypeStr;	// string of the function's parameter types for printing.
	AstNode *pFirstArgDeclNode; 	// a link list of DeclarationNode for each declaration in formal parameter list.
	AstNode *pFirstArgTypeNode;		// a link list of Typde Node for type of each formal parameter.
	AstNode *pFirstStatementNode; 	// a link list of StatementNode.
	AstNode *pReturnTypeNode;		// TypeNode pointer for scaler type, see CFG in parser.y
};

struct VariableRefNode {
	const char *pszVarName;
	AstNode *pFirstArrRefNode; 		// a link list of ExpressionNode.
	SymbolValue_t nVarType;			// filled by visit(), symbol value of the type of the referenced variable.
};

struct AssignNode {
	AstNode *pVariableRefNode;
	AstNode *pExpressionNode;
};

struct ReadNode {
	AstNode *pVariableRefNode;
};

struct ConditionNode {
	AstNode *pExpressionNode;
	AstNode *pThenCompoundStatementNode;
	AstNode *pElseCompoundStatementNode;
};

struct WhileNode {
	AstNode *pExpressionNode;
	AstNode *pCompoundStatementNode;
};

struct ReturnNode {
	AstNode *pExpressionNode;
};

struct ForNode {
	const char *pszLoopVar;
	int nStart;
	int nEnd;
	AstNode *pLoopVarNode;
	AstNode *pAssignSymbolNode;
	AstNode *pStartIntNode;
	AstNode *pEndIntNode;
	AstNode *pDeclarationNode;
	AstNode *pCompoundStatementNode;
};

// extern from jast.cpp
extern AstNode *NewAstNode(int nLine, int nCol, void *pBody, int (*funcPrint)(AstNode*, int), int (*funcVisit)(AstNode*), int (*funcCodeGen)(AstNode*));
extern AstNode *DupAstNode(AstNode *pAst);
extern AstNode *AddSiblingNode(AstNode *pNode, AstNode *pSibling);
extern void ErrorMessage(AstNode *pAst, const char *format, ...);
extern void PrintLeadingTabs(int nTab);
extern int  PrintAstNode(AstNode *pAst, int nLevel);
extern int  PrintAstList(AstNode *pFirstAst, int nLevel);
extern int  VisitAstNode(AstNode *pAst);
extern int  VisitAstList(AstNode *pFirstAst, bool bErrorBreak);
extern int  CodeGenAstNode(AstNode *pAst);
extern int  CodeGenAstList(AstNode *pFirstAst);
extern int  AstLinkLength(AstNode *pFirstNode);
extern int  SearchStringItem(const char *pszItem, const char *ppszItems[]);
extern bool InSymbolValueSet(SymbolValue_t nSymbol, const SymbolValue_t pnSymbols[]);
extern SymbolValue_t GetSymbolValue(const char *pszSymbol);
extern const char *GetSymbolString(SymbolValue_t n);
extern const char *GetArrayTypeString(AstNode *pAst);

// extern from SymTab.cpp
extern void SymTab_Init();
extern void SymTab_Release();
extern int  SymTab_Push();
extern int  SymTab_Pop();
extern int  SymTab_Insert(const char *pszName, const char *pszKind, const char *pszScalerType, const char *pszTypeStr, const char *pszAttr, AstNode *pAst);
extern int  SymTab_Lookup(const char *pszName);
extern int 	SymTab_GetLevel(int n);
extern const char *SymTab_GetName(int n);
extern const char *SymTab_GetKind(int n);
extern const char *SymTab_GetScalerType(int n);
extern const char *SymTab_GetTypeStr(int n);
extern const char *SymTab_GetAttr(int n);
extern SymbolValue_t SymTab_GetKindValue(int n);
extern SymbolValue_t SymTab_GetTypeValue(int n);
extern AstNode *SymTab_GetAstNode(int n);
extern void SymTab_Dump();
extern int 	SymTab_GetCurrStackLevel();

#endif //__JAST_INTERNAL_H__
