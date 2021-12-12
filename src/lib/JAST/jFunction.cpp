#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// Declare a global variable to show what function name currently in. 
AstNode *gCurrentFuncName = NULL;
extern AstNode *NewScalerTypeNode(int nLine, int nCol, const char *pszType);

// ----------------------------------------------------------------
// Print Funtion and FunctionInvocation node.
// ----------------------------------------------------------------
int PrintFunctionInvocationNode(AstNode *pAst, int nLevel)
{
	FunctionInvocationNode *pNode = (FunctionInvocationNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("function invocation <line: %d, col: %d> %s\n", pAst->location.line, pAst->location.col, pNode->pszFuncName);
	PrintAstList(pNode->pFirstExpressionNode, nLevel + 1);
	return 0;
}

int PrintFunctionNode(AstNode *pAst, int nLevel)
{	
	FunctionNode *pNode = (FunctionNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("function declaration <line: %d, col: %d> %s %s %s\n", pAst->location.line, pAst->location.col, pNode->pszFuncName, pNode->pszReturnType, pNode->pszParamTypeStr);
	PrintAstList(pNode->pFirstArgDeclNode, nLevel + 1);
	PrintAstList(pNode->pFirstStatementNode, nLevel + 1);
	return 0;
}

// ----------------------------------------------------------------
// Visit Function Node.
// ----------------------------------------------------------------
int VisitFunctionInvocationNode(AstNode *pAst)
{
	FunctionInvocationNode *pNode = (FunctionInvocationNode *)pAst->pBody;
	FunctionNode *pFunc;
	AstNode *p, *q;
	SymbolValue_t nArgType, nParamType;
	int n, nErr = 0;
	

	// 1. The identifier has to be in symbol tables.
	if ((n = SymTab_Lookup(pNode->pszFuncName)) < 0){
		ErrorMessage(pAst, "use of undeclared symbol '%s'\n", pNode->pszFuncName);
		return 1;
	}

	// 2. The kind of symbol has to be function..
	if (SymTab_GetKindValue(n) != kFunction){
		ErrorMessage(pAst, "call of non-function symbol '%s'\n", pNode->pszFuncName);
		return 1;
	}

	// After making sure the var was declared, obtain the function's return type from symbol table.
	pNode->nReturnType = SymTab_GetTypeValue(n);

	// 3. The number of arguments must be the same as one of the parameters.
	pFunc = (FunctionNode *)SymTab_GetAstNode(n)->pBody;
	if (AstLinkLength(pNode->pFirstExpressionNode) != AstLinkLength(pFunc->pFirstArgTypeNode)){
		ErrorMessage(pAst, "too few/much arguments provided for function '%s'\n", pNode->pszFuncName);
		return 1;
	}

	// 4. Traverse arguments: (Skip the rest of semantic checks if there are any errors in the node of the expression (argument))
	if ((nErr = VisitAstList(pNode->pFirstExpressionNode, true)) == 0){
		// 4.1 The type of the result of the expression (argument) must be the same type of the corresponding parameter after appropriate type coercion. 
		p = pNode->pFirstExpressionNode;
		q = pFunc->pFirstArgTypeNode;
		while(p && q){
			nArgType = ((ExpressionNode *)p->pBody)->nResultType;
			nParamType = ((TypeNode *)q->pBody)->nScalerType;
			if(!((nArgType == kInteger && nParamType == kReal) || nArgType == nParamType)){
				ErrorMessage(p, "incompatible type passing '%s' to parameter of type '%s'\n", GetSymbolString(nArgType), GetSymbolString(nParamType));
				return 1;
			}
			p = p->pNext;
			q = q->pNext;
		}
	}
	
	return nErr;
}

int VisitFunctionNode(AstNode *pAst)
{
	FunctionNode *pNode = (FunctionNode *)pAst->pBody;
	int n, nErr = 0;

	// Get function name currently in.
	gCurrentFuncName = pAst;

	n = SymTab_Lookup(pNode->pszFuncName);
	if (n >= 0 && SymTab_GetLevel(n) == SymTab_GetCurrStackLevel()){
		ErrorMessage(pAst, "symbol '%s' is redeclared\n", pNode->pszFuncName); // DBG : not quite sure
		nErr++;
	}
	else{
		SymTab_Insert(pNode->pszFuncName, "function", pNode->pszReturnType, pNode->pszReturnType, pNode->pszParamTypeStr, pAst);
	}

	// For function definition, pFirstStatementNode != NULL, push symbol table and visit args and statements.
	if (pNode->pFirstStatementNode){
		SymTab_Push();
		nErr += VisitAstList(pNode->pFirstArgDeclNode, false);
		nErr += VisitAstList(pNode->pFirstStatementNode, false);
		SymTab_Pop();
	}

	// Set the gCurrentFuncName to NULL when leaving the function.
	gCurrentFuncName = NULL;

	return nErr;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewFunctionInvocationNode(int nLine, int nCol, const char *pszFuncName, AstNode *pFirstExpressionNode)
{
	// Filling in body contents.
	FunctionInvocationNode *pBody = new FunctionInvocationNode;
	pBody->pszFuncName = strdup(pszFuncName);
	pBody->pFirstExpressionNode = pFirstExpressionNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintFunctionInvocationNode, VisitFunctionInvocationNode, NULL);
}

AstNode *NewFunctionNode(int nLine, int nCol, const char *pszFuncName, AstNode *pFirstArgDeclNode, AstNode *pReturnTypeNode, AstNode *pFirstStatementNode)
 {
 	int i, n;
	char pszTemp[256];
	AstNode *p, oNode;
	DeclarationNode *pDecl;

	if (!pReturnTypeNode)
		pReturnTypeNode = NewScalerTypeNode(nLine, 0, "void");

	// Filling in body contents.
 	FunctionNode *pBody = new FunctionNode;
 	pBody->pszFuncName = strdup(pszFuncName);
	pBody->pszReturnType = ((TypeNode *)pReturnTypeNode->pBody)->pszScalerType;
	pBody->pFirstArgDeclNode = pFirstArgDeclNode;
	pBody->pReturnTypeNode = pReturnTypeNode;
	pBody->pFirstStatementNode = pFirstStatementNode;

	// Expand the declaration and generate formal parameters' type list.
	oNode.pNext = NULL;
	p = pFirstArgDeclNode;
	while(p){
		pDecl = (DeclarationNode *)p->pBody;
		for(i = 0; i < AstLinkLength(pDecl->pFirstIdNode); i++)
			AddSiblingNode(&oNode, DupAstNode(pDecl->pTypeNode));
		p = p->pNext;
	}
	pBody->pFirstArgTypeNode = oNode.pNext;

	// Generate string that needs to be printed for formal parameters types.
	strcpy(pszTemp, "(");
	p = pBody->pFirstArgTypeNode;
	while(p){
		strcat(pszTemp, ((TypeNode *)p->pBody)->pszTypeStr);
		strcat(pszTemp, ", ");			
		p = p->pNext;
	}
	n = strlen(pszTemp);
	if (n > 1)
		pszTemp[n - 2] = 0;
	strcat(pszTemp, ")");
	pBody->pszParamTypeStr = strdup(pszTemp);

	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintFunctionNode, VisitFunctionNode, NULL);
 }
