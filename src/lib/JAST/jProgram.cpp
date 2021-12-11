#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// ----------------------------------------------------------------
// Print functions for each non-terminal
// ----------------------------------------------------------------
int PrintProgramNode(AstNode *pAst, int nLevel)
{
	ProgramNode *pNode = (ProgramNode *)pAst->pBody;

	PrintLeadingTabs(nLevel);
	printf("program <line: %d, col: %d> %s void\n", pAst->location.line, pAst->location.col, pNode->pszName);
	PrintAstList(pNode->pFirstDeclarationNode, nLevel + 1);
	PrintAstList(pNode->pFirstFunctionNode, nLevel + 1);
	PrintAstNode(pNode->pCompoundStatementNode, nLevel + 1);
	return 0;
}

// ----------------------------------------------------------------
// Visit Declaration related Node.
// ----------------------------------------------------------------
int VisitProgramNode(AstNode *pAst)
{
	ProgramNode *pNode = (ProgramNode *)pAst->pBody;
	int nErr = 0;

	SymTab_Push();
	SymTab_Insert(pNode->pszName, "program", "", "", "", pAst);
	nErr += VisitAstList(pNode->pFirstDeclarationNode, false);
	nErr += VisitAstList(pNode->pFirstFunctionNode, false);
	nErr += VisitAstNode(pNode->pCompoundStatementNode);
	SymTab_Pop();
	return nErr;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewProgramNode(int nLine, int nCol, const char *pszProgName, AstNode *pFirstDeclarationNode, AstNode *pFirstFunctionNode, AstNode *pCompoundStatementNode)
{
	// Filling in body contents.
	ProgramNode *pBody = new ProgramNode;
	pBody->pszName = strdup(pszProgName);
	pBody->pFirstDeclarationNode = pFirstDeclarationNode;
	pBody->pFirstFunctionNode = pFirstFunctionNode;
	pBody->pCompoundStatementNode = pCompoundStatementNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintProgramNode, VisitProgramNode, NULL);
}


