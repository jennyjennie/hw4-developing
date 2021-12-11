#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

// ----------------------------------------------------------------
//  Print Expression Node.
// ----------------------------------------------------------------
int PrintExpressionNode(AstNode *pAst, int nLevel)
{
	ExpressionNode *pNode = (ExpressionNode *)pAst->pBody;
	const char *kTerminalOps[] = {"constant", "VariableReference", "FunctionInvocation", NULL};
	int n;
	
	if (!pNode->pLeftNode){
		PrintLeadingTabs(nLevel);
		printf("unary operator <line: %d, col: %d> %s\n", pAst->location.line, pAst->location.col, pNode->pszOp);
		PrintAstNode(pNode->pRightNode, nLevel + 1);
	}
	else if (!pNode->pRightNode){
		n = SearchStringItem(pNode->pszOp, kTerminalOps);
		if (kTerminalOps[n])
			PrintAstNode(pNode->pLeftNode, nLevel);
	}
	else{
		PrintLeadingTabs(nLevel);
		printf("binary operator <line: %d, col: %d> %s\n", pAst->location.line, pAst->location.col, pNode->pszOp);
		PrintAstNode(pNode->pLeftNode, nLevel + 1);
		PrintAstNode(pNode->pRightNode, nLevel + 1);
	}
	return 0;
}

// ----------------------------------------------------------------
//  Determine Expression Node type after calculation.
// ----------------------------------------------------------------
int determine_op_type(AstNode *pAst)
{
	ExpressionNode *pNode = (ExpressionNode *)pAst->pBody;
	ExpressionNode *pLeft = pNode->pLeftNode ? (ExpressionNode *)pNode->pLeftNode->pBody: NULL;
	ExpressionNode *pRight = pNode->pRightNode ? (ExpressionNode *)pNode->pRightNode->pBody : NULL;

	const SymbolValue_t kIntReal[] = {kInteger, kReal, kUnknown};
	const SymbolValue_t kArithmetics[] = {kADD, kMINUS, kMULTIPLY, kDIVIDE, kUnknown};
	const SymbolValue_t kRelations[] = {kLT, kLE, kEQ, kGE, kGT, kNE, kUnknown};
	const SymbolValue_t kBooleans[] = {kAND, kOR, kUnknown};

	if (pNode->nOp == kADD && pLeft->nResultType == kString && pRight->nResultType == kString){
		pNode->nOp = kSTRCAT;
		pNode->nResultType = kString;
	}
	else if (pNode->nOp == kNEG){
		if (InSymbolValueSet(pRight->nResultType, kIntReal))
			pNode->nResultType = pRight->nResultType;
		else
			printf("%d:%d: error: The operand of unary '%s' must be integer or real.\n", pAst->location.line, pAst->location.col, pNode->pszOp); //DBG : Delete
	}
	else if (pNode->nOp == kNOT){
		if (pRight->nResultType == kBoolean)
			pNode->nResultType = kBoolean;
		else
			printf("%d:%d: error: The operand of unary '%s' must be boolean.\n", pAst->location.line, pAst->location.col, pNode->pszOp); //DBG : Delete
	}
	else if (pNode->nOp == kMOD){
		if (pLeft->nResultType == kInteger && pRight->nResultType == kInteger)
			pNode->nResultType = kInteger;
		else
			printf("%d:%d: error: Operands of '%s' must be integers.\n", pAst->location.line, pAst->location.col, pNode->pszOp); //DBG : Delete
	}
	else if (InSymbolValueSet(pNode->nOp, kArithmetics)){
		if (InSymbolValueSet(pLeft->nResultType, kIntReal) && InSymbolValueSet(pRight->nResultType, kIntReal))
			pNode->nResultType = (pLeft->nResultType == kReal || pRight->nResultType == kReal) ? kReal : kInteger;
		else
			printf("%d:%d: error: Operands of '%s' must be integer or real.\n", pAst->location.line, pAst->location.col, pNode->pszOp); //DBG : Delete
	}
	else if (InSymbolValueSet(pNode->nOp, kRelations)){
		if (InSymbolValueSet(pLeft->nResultType, kIntReal) && InSymbolValueSet(pRight->nResultType, kIntReal))
			pNode->nResultType = kBoolean;
		else
			printf("%d:%d: error: Operands of '%s' must be integer or real.\n", pAst->location.line, pAst->location.col, pNode->pszOp); 
	}
	else if (InSymbolValueSet(pNode->nOp, kBooleans)){
		if (pLeft->nResultType == kBoolean && pRight->nResultType == kBoolean)
			pNode->nResultType = kBoolean;
		else
			printf("%d:%d: error: Operands of '%s' must be boolean.\n", pAst->location.line, pAst->location.col, pNode->pszOp);
	}

	return (pNode->nResultType != kUnknown) ? 0 : 1;
}

int VisitExpressionNode(AstNode *pAst)
{
	ExpressionNode *pNode = (ExpressionNode *)pAst->pBody;
	int nErr = 0;
	
	nErr += VisitAstNode(pNode->pLeftNode);
	nErr += VisitAstNode(pNode->pRightNode);

	if (nErr == 0){
		if (strcmp(pNode->pszOp, "constant") == 0)
			pNode->nResultType = ((LiteralNode *)pNode->pLeftNode->pBody)->nType;
		else if (strcmp(pNode->pszOp, "VariableReference") == 0)
			pNode->nResultType = ((VariableRefNode *)pNode->pLeftNode->pBody)->nVarType;
		else if (strcmp(pNode->pszOp, "FunctionInvocation") == 0)
			pNode->nResultType = ((FunctionInvocationNode *)pNode->pLeftNode->pBody)->nReturnType;
		else
			nErr = determine_op_type(pAst);
	}

	return nErr;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewExpressionNode(int nLine, int nCol, const char *pszOp, AstNode *pLeftNode, AstNode *pRightNode)
{
	// Filling in body contents.
	ExpressionNode *pBody = new ExpressionNode;
	pBody->pszOp = pszOp;
	pBody->pLeftNode = pLeftNode;
	pBody->pRightNode = pRightNode;
	pBody->nOp = GetSymbolValue(pBody->pszOp);
	pBody->nResultType = kUnknown;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintExpressionNode, VisitExpressionNode, NULL);
}


