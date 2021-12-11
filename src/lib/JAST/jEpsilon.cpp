#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// Epsilon node: only used for debug at development stage.
struct EpsilonNode {
	const char *pszPrefix;
	const char *pszPostfix;
};

int PrintEpsilonNode(AstNode *pAst, int nLevel)
{
	EpsilonNode *pNode = (EpsilonNode *)pAst->pBody;
	PrintLeadingTabs(nLevel);
	printf("%s <line: %d, col: %d> %s\n", pNode->pszPrefix, pAst->location.line, pAst->location.col, pNode->pszPostfix);
	return 0;
}

AstNode *NewEpsilonNode(int nLine, int nCol, const char *pszPrefix, const char *pszPostfix)
{
	EpsilonNode *p = new EpsilonNode;

	p->pszPrefix = strdup(pszPrefix);
	p->pszPostfix = strdup(pszPostfix);
	return NewAstNode(nLine, nCol, p, PrintEpsilonNode, NULL, NULL);
}
