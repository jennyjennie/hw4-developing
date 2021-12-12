#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAST/jast_internal.h"

extern AstNode *gCurrentFuncNode;
extern AstNode *NewDeclarationNode_Type(int nLine, int nCol, AstNode *pFirstIdNode, AstNode *pTypeNode, const char *pszKind);
extern AstNode *NewScalerTypeNode(int nLine, int nCol, const char *pszType);

// ----------------------------------------------------------------
// Print Condition, While, Return and For Node.
// ----------------------------------------------------------------
int PrintConditionNode(AstNode *pAst, int nLevel)
{
	ConditionNode *pNode = (ConditionNode *)pAst->pBody;

	PrintLeadingTabs(nLevel);
	printf("if statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pExpressionNode, nLevel + 1);
	PrintAstNode(pNode->pThenCompoundStatementNode, nLevel + 1);
	PrintAstNode(pNode->pElseCompoundStatementNode, nLevel + 1);
	return 0;
}

int PrintWhileNode(AstNode *pAst, int nLevel)
{
	WhileNode *pNode = (WhileNode *)pAst->pBody;

	PrintLeadingTabs(nLevel);
	printf("while statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pExpressionNode, nLevel + 1);
	PrintAstNode(pNode->pCompoundStatementNode, nLevel + 1);
	return 0;
}

int PrintReturnNode(AstNode *pAst, int nLevel)
{
	ReturnNode *pNode = (ReturnNode *)pAst->pBody;

	PrintLeadingTabs(nLevel);
	printf("return statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pExpressionNode, nLevel + 1);
	return 0;
}

int PrintForNode(AstNode *pAst, int nLevel)
{
	ForNode *pNode = (ForNode *)pAst->pBody;
	Location loc2, loc3, loc4, loc6;

	loc2 = pNode->pLoopVarNode->location;
	loc3 = pNode->pAssignSymbolNode->location;
	loc4 = pNode->pStartIntNode->location;
	loc6 = pNode->pEndIntNode->location;

	PrintLeadingTabs(nLevel);
	printf("for statement <line: %d, col: %d>\n", pAst->location.line, pAst->location.col);
	PrintAstNode(pNode->pDeclarationNode, nLevel + 1); 
	PrintLeadingTabs(nLevel + 1);
	printf("assignment statement <line: %d, col: %d>\n", loc3.line, loc3.col);
	PrintLeadingTabs(nLevel + 2);
	printf("variable reference <line: %d, col: %d> %s\n", loc2.line, loc2.col, pNode->pszLoopVar);
	PrintLeadingTabs(nLevel + 2);
	printf("constant <line: %d, col: %d> %d\n", loc4.line, loc4.col, pNode->nStart);
	PrintLeadingTabs(nLevel + 1);
	printf("constant <line: %d, col: %d> %d\n", loc6.line, loc6.col, pNode->nEnd);
	PrintAstNode(pNode->pCompoundStatementNode, nLevel + 1);
	return 0;
}

// ----------------------------------------------------------------
// Visit and semantically check Condition, While, Return, For nodes 
// ----------------------------------------------------------------
int VisitConditionNode(AstNode *pAst)
{
	int nErr = 0;
	SymbolValue_t nResultType;
	ConditionNode *pNode = (ConditionNode *)pAst->pBody;
	// Skip further checks if there is semantic errors in expression.
	if ((nErr = VisitAstNode(pNode->pExpressionNode)) == 0){
		nResultType = ((ExpressionNode *)(pNode->pExpressionNode)->pBody)->nResultType;
		// // Skip further checks if the result type of expression is not boolean.
		if(nResultType != kBoolean){
			ErrorMessage(pNode->pExpressionNode, "the expression of condition must be boolean type\n");
			nErr++;
		}
	}
	else{
		nErr += VisitAstNode(pNode->pThenCompoundStatementNode);
		nErr += VisitAstNode(pNode->pElseCompoundStatementNode);
	}
	
	return nErr;
}

int VisitWhileNode(AstNode *pAst)
{
	int nErr = 0;
	SymbolValue_t nResultType;
	WhileNode *pNode = (WhileNode *)pAst->pBody;

	// Skip further checks if there is semantic errors in expression.
	if ((nErr = VisitAstNode(pNode->pExpressionNode)) == 0){
		nResultType = ((ExpressionNode *)(pNode->pExpressionNode)->pBody)->nResultType;
		// If the result type of the expression is not boolean, then no need to do further checks.
		if(nResultType != kBoolean){
			ErrorMessage(pNode->pExpressionNode, "the expression of condition must be boolean type\n");
			nErr++;
		}
	}
	else{
		nErr += VisitAstNode(pNode->pCompoundStatementNode);
	}
	return nErr;
}

int VisitReturnNode(AstNode *pAst)
{
	int nErr = 0;
	const char *pszFuncType, *pszResultType;
	ReturnNode *pNode = (ReturnNode *)pAst->pBody;

	// 1. Check if currently in the main program. (not in any function)
	if(!gCurrentFuncNode){
		ErrorMessage(pAst, "program/procedure should not return a value\n");
		nErr++;
	}
	else{
		// To-do : get the type of expression if it is array type.
		// 2. Skip further checks if there is semantic errors in expressions.
		if((nErr = VisitAstNode(pNode->pExpressionNode)) == 0){
			pszFuncType = ((FunctionNode *)gCurrentFuncNode->pBody)->pszReturnType;
			pszResultType = GetSymbolString(((ExpressionNode *)(pNode->pExpressionNode)->pBody)->nResultType);
			pszResultType  = pszResultType ? pszResultType : GetArrayTypeString(pNode->pExpressionNode);
			// 2.1 Check if the function return type is "void".
			if(strcmp(pszFuncType, "void") == 0){
				ErrorMessage(pAst, "program/procedure should not return a value\n");
				nErr++;
			}
			// 2.2 Check if the return type is the same as the function return type after type coercion.
			else if(!(strcmp(pszFuncType, pszResultType) == 0 || (strcmp(pszFuncType, "real") == 0 && strcmp(pszResultType, "integer") == 0))){
				ErrorMessage(pAst, "return '%s' from a function with return type '%s'\n", pszFuncType, pszResultType);
				nErr++;
			}
		}
	}

	return nErr;
}

int VisitForNode(AstNode *pAst)
{
	int nErr = 0;
	int n1, n2;
	ForNode *pNode = (ForNode *)pAst->pBody;

	SymTab_Push();
	// Make sure the lower bound and upper bound of iteration count be in the incremental order
	n1 = ((IntValueNode *)(pNode->pStartIntNode)->pBody)->nValue;
	n2 = ((IntValueNode *)(pNode->pEndIntNode)->pBody)->nValue;
	if (n1 > n2){
		ErrorMessage(pAst, "the lower bound and upper bound of iteration count must be in the incremental order\n");
		nErr++;
	}
	nErr += VisitAstNode(pNode->pDeclarationNode);
	nErr += VisitAstNode(pNode->pCompoundStatementNode);
	SymTab_Pop();
	
	return nErr;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewConditionNode(int nLine, int nCol, AstNode *pExpressionNode, AstNode *pThenCompoundStatementNode, AstNode *pElseCompoundStatementNode)
{
	// Filling in body contents.
	ConditionNode *pBody = new ConditionNode;
	pBody->pExpressionNode = pExpressionNode;
	pBody->pThenCompoundStatementNode = pThenCompoundStatementNode;
	pBody->pElseCompoundStatementNode = pElseCompoundStatementNode;	// It can be NULL.
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintConditionNode, VisitConditionNode, NULL);
}

AstNode *NewWhileNode(int nLine, int nCol, AstNode *pExpressionNode, AstNode *pCompoundStatementNode)
{
	// Filling in body contents.
	WhileNode *pBody = new WhileNode;
	pBody->pExpressionNode = pExpressionNode;
	pBody->pCompoundStatementNode = pCompoundStatementNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintWhileNode, VisitWhileNode, NULL);
}

AstNode *NewReturnNode(int nLine, int nCol, AstNode *pExpressionNode)
{
	// Filling in body contents.
	ReturnNode *pBody = new ReturnNode;
	pBody->pExpressionNode = pExpressionNode;
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintReturnNode, VisitReturnNode, NULL);
}

AstNode *NewForNode(int nLine, int nCol, AstNode *pLoopVarNode, AstNode *pAssignSymbolNode, AstNode *pStartIntNode, AstNode *pEndIntNode, AstNode *pCompoundStatementNode)
{
	Location loc;

	// Filling in body contents.
	ForNode *pBody = new ForNode;
	pBody->pszLoopVar = ((IdNode *)pLoopVarNode->pBody)->pszName;
	pBody->nStart = ((IntValueNode *)pStartIntNode->pBody)->nValue;
	pBody->nEnd = ((IntValueNode *)pEndIntNode->pBody)->nValue;
	pBody->pLoopVarNode = pLoopVarNode;
	pBody->pAssignSymbolNode = pAssignSymbolNode;
	pBody->pStartIntNode = pStartIntNode;
	pBody->pEndIntNode = pEndIntNode;
	pBody->pCompoundStatementNode = pCompoundStatementNode;
	loc = pLoopVarNode->location;
	pBody->pDeclarationNode = NewDeclarationNode_Type(loc.line, loc.col, pLoopVarNode, NewScalerTypeNode(loc.line, loc.col, "integer"), "loop_var");
	// Build AST.
	return NewAstNode(nLine, nCol, pBody, PrintForNode, VisitForNode, NULL);
}
