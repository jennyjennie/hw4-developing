#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewIdNode(int nLine, int nCol, const char *pszName)
{
	// Filling in body contents.
	IdNode *pBody = new IdNode;
	pBody->pszName = strdup(pszName);
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, NULL, NULL, NULL);
}

AstNode *NewIntValueNode(int nLine, int nCol, int n)
{
	// Filling in body contents.
	IntValueNode *pBody = new IntValueNode;
	pBody->nValue = n;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, NULL, NULL, NULL);
}

AstNode *NewScalerTypeNode(int nLine, int nCol, const char *pszType)
{
	// Filling in body contents.
	TypeNode *pBody = new TypeNode;
	pBody->pszScalerType = strdup(pszType);
	pBody->nScalerType = GetSymbolValue(pszType);
	pBody->pFirstIntNode = NULL;
	pBody->pszTypeStr = pBody->pszScalerType;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, NULL, NULL, NULL);
}

AstNode *NewArrTypeNode(int nLine, int nCol, AstNode *pFirstIntNode, const char *pszType)
{
	AstNode *p;
	char pszTemp[256], pszStr[256];

	// Filling in body contents.
	TypeNode *pBody = new TypeNode;
	pBody->pszScalerType = strdup(pszType);
	pBody->nScalerType = GetSymbolValue(pszType);
	pBody->pFirstIntNode = pFirstIntNode;
	p = pFirstIntNode;
	strcpy(pszStr, pszType);
	strcat(pszStr, " ");
	while(p){
		sprintf(pszTemp, "[%d]", ((IntValueNode *)p->pBody)->nValue);
		strcat(pszStr, pszTemp);
		p = p->pNext;
	}
	pBody->pszTypeStr = strdup(pszStr);
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, NULL, NULL, NULL);
}

