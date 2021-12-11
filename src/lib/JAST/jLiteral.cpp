#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// ----------------------------------------------------------------
// Print Literal Node.
// ----------------------------------------------------------------
int PrintLiteralNode(AstNode *pAst, int nLevel)
{
	LiteralNode *pNode = (LiteralNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("constant <line: %d, col: %d> %s\n", pAst->location.line, pAst->location.col, pNode->pszStr);
	return 0;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewLiteralIntNode(int nLine, int nCol, int nValue)
{
	char pszTemp[256];

	// Filling in body contents.
	LiteralNode *pBody = new LiteralNode;
	pBody->pszType = "integer";
	pBody->nType = kInteger;
	pBody->nLiteralInt = nValue;
	sprintf(pszTemp, "%d", nValue);
	pBody->pszStr = strdup(pszTemp);
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintLiteralNode, NULL, NULL);
}

AstNode *NewLiteralRealNode(int nLine, int nCol, double dValue)
{
	char pszTemp[256];

	// Filling in body contents.
	LiteralNode *pBody = new LiteralNode;
	pBody->pszType = "real";
	pBody->nType = kReal;
	pBody->dLiteralReal = dValue;
	sprintf(pszTemp, "%.6lf", dValue);
	pBody->pszStr = strdup(pszTemp);
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintLiteralNode, NULL, NULL);
}

AstNode *NewLiteralStringNode(int nLine, int nCol, const char *pszStr)
{
	// Filling in body contents.
	LiteralNode *pBody = new LiteralNode;
	pBody->pszType = "string";
	pBody->nType = kString;
	pBody->pszLiteralString = strdup(pszStr);
	pBody->pszStr = pBody->pszLiteralString;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintLiteralNode, NULL, NULL);
}

AstNode *NewLiteralBooleanNode(int nLine, int nCol, bool nBoolean)
{
	// Filling in body contents.
	LiteralNode *pBody = new LiteralNode;
	pBody->pszType = "boolean";
	pBody->nType = kBoolean;
	pBody->nLiteralBoolean = nBoolean;
	pBody->pszStr = nBoolean ? "true" : "false";
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintLiteralNode, NULL, NULL);
}

