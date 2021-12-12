#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

extern AstNode *NewScalerTypeNode(int nLine, int nCol, const char *pszType);

// ----------------------------------------------------------------
// Print Declaration related Node.
// ----------------------------------------------------------------
int PrintDeclarationNode(AstNode *pAst, int nLevel)
{
	AstNode *p;
	DeclarationNode *pNode = (DeclarationNode *)pAst->pBody;
	TypeNode *pType = (TypeNode *)pNode->pTypeNode->pBody;

	PrintLeadingTabs(nLevel);
	printf("declaration <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	p = pNode->pFirstIdNode;
	while(p){
		PrintLeadingTabs(nLevel + 1);
		printf("variable <line: %d, col: %d> %s %s\n", p->location.line, p->location.col, ((IdNode *)p->pBody)->pszName, pType->pszTypeStr);
		if (pNode->pLiteralNode)
			PrintAstNode(pNode->pLiteralNode, nLevel + 2);
		p = p->pNext;
	}
	return 0;
}

// ----------------------------------------------------------------
// Visit Declaration related Node.
// ----------------------------------------------------------------
int VisitDeclarationNode(AstNode *pAst)
{
	AstNode *p, *q;
	DeclarationNode *pNode = (DeclarationNode *)pAst->pBody;
	TypeNode *pType = (TypeNode *)pNode->pTypeNode->pBody;
	LiteralNode *pLiteral = pNode->pLiteralNode ? (LiteralNode *)pNode->pLiteralNode->pBody : NULL;
	const char *pszName;
	int n, nErr = 0;

	p = pNode->pFirstIdNode;
	while(p){
		pszName = ((IdNode *)p->pBody)->pszName;
		n = SymTab_Lookup(pszName);
		if (n >= 0 && SymTab_GetLevel(n) == SymTab_GetCurrStackLevel()){
			ErrorMessage(p, "symbol '%s' is redeclared\n", pszName);
			nErr++;
		}
		else if (n >= 0 && SymTab_GetKindValue(n) == kLoopVar){
			ErrorMessage(p, "symbol '%s' is redeclared\n", pszName);
			nErr++;
		}
		else{
			SymTab_Insert(pszName, pNode->pszKind, pType->pszScalerType, pType->pszTypeStr, pLiteral ? pLiteral->pszStr : "", pNode->pTypeNode);
		}
		p = p->pNext;
	}

	// Get the type node, if it has a list of int nodes, then mask sure each int > 0.
	q = pType->pFirstIntNode;
	p = pNode->pFirstIdNode;
	while(q){
		if(((IntValueNode *)q->pBody)->nValue <= 0){
			ErrorMessage(p, "%s declared as an array with an index that is not greater than 0\n", ((IdNode *)p->pBody)->pszName);
			nErr++;
			break;
		}
		q = q->pNext;
	}

	return nErr;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewDeclarationNode_Type(int nLine, int nCol, AstNode *pFirstIdNode, AstNode *pTypeNode, const char *pszKind)
{
	// Filling in body contents.
	DeclarationNode *pBody = new DeclarationNode;
	pBody->pszKind = pszKind;
	pBody->nKind = GetSymbolValue(pszKind);
	pBody->pFirstIdNode = pFirstIdNode;
	pBody->pTypeNode = pTypeNode;
	pBody->pLiteralNode = NULL;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintDeclarationNode, VisitDeclarationNode, NULL);
}

AstNode *NewDeclarationNode_LiteralConstant(int nLine, int nCol, AstNode *pFirstIdNode, AstNode *pLiteralNode)
{
	// Filling in body contents.
	DeclarationNode *pBody = new DeclarationNode;
	pBody->pszKind = "constant";
	pBody->nKind = kConstant;
	pBody->pFirstIdNode = pFirstIdNode;
	pBody->pTypeNode = NewScalerTypeNode(nLine, nCol, ((LiteralNode *)pLiteralNode->pBody)->pszType);
	pBody->pLiteralNode = pLiteralNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintDeclarationNode, VisitDeclarationNode, NULL);
}
