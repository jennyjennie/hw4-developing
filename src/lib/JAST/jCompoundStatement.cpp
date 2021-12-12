#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// ----------------------------------------------------------------
// Print CompoundStatement related Node.
// ----------------------------------------------------------------
int PrintCompoundStatementNode(AstNode *pAst, int nLevel)
{
	CompoundStatementNode *pNode = (CompoundStatementNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("compound statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstList(pNode->pFirstDeclarationNode, nLevel + 1);
	PrintAstList(pNode->pFirstStatementNode, nLevel + 1);
	return 0;
}

int PrintPrintNode(AstNode *pAst, int nLevel)
{
	PrintNode *pNode = (PrintNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("print statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pExpressionNode, nLevel + 1);
	return 0;
}

int PrintVariableRefNode(AstNode *pAst, int nLevel)
{
	VariableRefNode *pNode = (VariableRefNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("variable reference <line: %d, col: %d> %s\n", pAst->location.line, pAst->location.col, pNode->pszVarName);
	PrintAstList(pNode->pFirstArrRefNode, nLevel + 1);
	return 0;
}

int PrintAssignNode(AstNode *pAst, int nLevel)
{
	AssignNode *pNode = (AssignNode *)pAst->pBody;

	PrintLeadingTabs(nLevel);
	printf("assignment statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pVariableRefNode, nLevel + 1);
	PrintAstNode(pNode->pExpressionNode, nLevel + 1);

	return 0;
}

int PrintReadNode(AstNode *pAst, int nLevel)
{
	ReadNode *pNode = (ReadNode *)pAst->pBody;

	PrintLeadingTabs(nLevel);
	printf("read statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pVariableRefNode, nLevel + 1);		
	
	return 0;
}

// ----------------------------------------------------------------
// Visit CompoundStatement related Node.
// ----------------------------------------------------------------
int VisitCompoundStatementNode(AstNode *pAst)
{
	CompoundStatementNode *pNode = (CompoundStatementNode *)pAst->pBody;
	int nErr = 0;

	SymTab_Push();
	nErr += VisitAstList(pNode->pFirstDeclarationNode, false);
	nErr += VisitAstList(pNode->pFirstStatementNode, false);
	SymTab_Pop();
	return nErr;
}

int VisitPrintNode(AstNode *pAst)
{
	int nErr;
	SymbolValue_t nResultType;
	PrintNode *pNode = (PrintNode *)pAst->pBody;
	// Check whether the type of the expression (target) is scalar type.
	if ((nErr = VisitAstNode(pNode->pExpressionNode)) == 0){
		nResultType = ((ExpressionNode *)(pNode->pExpressionNode)->pBody)->nResultType;
		if(nResultType != kInteger && nResultType != kReal && nResultType != kBoolean && nResultType != kString){
			ErrorMessage(pNode->pExpressionNode, "expression of print statement must be scalar type\n");
			nErr++;
		}
	}
	return nErr;
}

int VisitVariableRefNode(AstNode *pAst)
{
	VariableRefNode *pNode = (VariableRefNode *)pAst->pBody;
	AstNode *p;
	int n, nKind, nErr, nDimRef, nDimDecl;

	// 1. The identifier has to be in symbol tables.
	if ((n = SymTab_Lookup(pNode->pszVarName)) < 0){
		ErrorMessage(pAst, "use of undeclared symbol '%s'\n", pNode->pszVarName);
		return 1;
	}

	// 2. The kind of symbol has to be a parameter, variable, loop_var, or constant.
	nKind = SymTab_GetKindValue(n);
	if (nKind != kParameter && nKind != kVariable && nKind != kLoopVar && nKind != kConstant){
		ErrorMessage(pAst, "use of non-variable symbol '%s'\n", pNode->pszVarName);
		return 1;
	}

	// Continue visit expression in each dimension of array reference.
	if ((nErr = VisitAstList(pNode->pFirstArrRefNode, true)) > 0)
		return nErr;

	// 3. Each index of an array reference must be of the integer type.
	p = pNode->pFirstArrRefNode;
	while(p){
		if (((ExpressionNode *)p->pBody)->nResultType != kInteger){
			ErrorMessage(p, "index of array reference must be an integer\n");
			return 1;
		}
		p = p->pNext;
	}

	// 4. An over array subscript is forbidden, that is, the number of indices 
	// of an array reference cannot be greater than the one of dimensions in the declaration.
	p = SymTab_GetAstNode(n);
	nDimRef = AstLinkLength(pNode->pFirstArrRefNode);
	nDimDecl = AstLinkLength(((TypeNode *)p->pBody)->pFirstIntNode);
	if (nDimRef > nDimDecl){
		ErrorMessage(pAst, "there is an over array subscript on '%s'\n", pNode->pszVarName);
		return 1;
	}

	// After passing all checks, obtain the var type from symbol table when there is no under array subscript.
	pNode->nVarType = (nDimRef == nDimDecl) ? SymTab_GetTypeValue(n) : kUnknown;

	return 0;
}

int VisitAssignReadNode(AstNode *pAst)
{	
	int nErr = 0;
	
	return nErr;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewCompoundStatementNode(int nLine, int nCol, AstNode *pFirstDeclarationNode, AstNode *pFirstStatementNode)
{
	// Filling in body contents.
	CompoundStatementNode *pBody = new CompoundStatementNode;
	pBody->pFirstDeclarationNode = pFirstDeclarationNode;
	pBody->pFirstStatementNode = pFirstStatementNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintCompoundStatementNode, VisitCompoundStatementNode, NULL);
}

AstNode *NewPrintNode(int nLine, int nCol, AstNode *pExpressionNode)
{
	// Filling in body contents.
	PrintNode *pBody = new PrintNode;
	pBody->pExpressionNode = pExpressionNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintPrintNode, VisitPrintNode, NULL);
}

AstNode *NewVariableRefNode(int nLine, int nCol, const char *pszVarName, AstNode *pFirstArrRefNode)
{
	// Filling in body contents.
	VariableRefNode *pBody = new VariableRefNode;
	pBody->pszVarName = pszVarName;
	pBody->pFirstArrRefNode = pFirstArrRefNode;
	pBody->nVarType = kUnknown;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintVariableRefNode, VisitVariableRefNode, NULL);
}

AstNode *NewAssignNode(int nLine, int nCol, AstNode *pVariableRefNode, AstNode *pExpressionNode)
{
	// Filling in body contents.
	AssignNode *pBody = new AssignNode;
	pBody->pVariableRefNode = pVariableRefNode;
	pBody->pExpressionNode = pExpressionNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintAssignNode, NULL, NULL);
}

AstNode *NewReadNode(int nLine, int nCol, AstNode *pVariableRefNode)
{
	// Filling in body contents.
	ReadNode *pBody = new ReadNode;
	pBody->pVariableRefNode = pVariableRefNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintReadNode, NULL, NULL);
}