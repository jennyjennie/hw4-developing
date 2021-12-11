
#ifndef __JAST_API_H__
#define __JAST_API_H__

#include <stdio.h>
#include "jast.h"

// Tree construction functions for bison. 

// extern from jast.cpp
extern int PrintAstNode(AstNode *pAst, int nLevel);
extern int VisitAstNode(AstNode *pAst);
extern int CodeGenAstNode(AstNode *pAst);
extern AstNode *AddSiblingNode(AstNode *pNode, AstNode *pSibling);

// extern from jProgram.cpp
extern AstNode *NewProgramNode(int nLine, int nCol, const char *pszProgName, AstNode *pFirstDeclarationNode, AstNode *pFirstFunctionNode, AstNode *pCompoundStatementNode);

// extern from jDeclaration.cpp
extern AstNode *NewDeclarationNode_Type(int nLine, int nCol, AstNode *pFirstIdNode, AstNode *pTypeNode, const char *pszKind);
extern AstNode *NewDeclarationNode_LiteralConstant(int nLine, int nCol, AstNode *pFirstIdNode, AstNode *pLiteralNode);

// extern from jType.cpp
extern AstNode *NewScalerTypeNode(int nLine, int nCol, const char *pszType);
extern AstNode *NewArrTypeNode(int nLine, int nCol, AstNode *pFirstIntNode, const char *pszType);
extern AstNode *NewIdNode(int nLine, int nCol, const char *pszName);
extern AstNode *NewIntValueNode(int nLine, int nCol, int n);

// extern from jLiteral.cpp
extern AstNode *NewLiteralIntNode(int nLine, int nCol, int nValue);
extern AstNode *NewLiteralRealNode(int nLine, int nCol, double dValue);
extern AstNode *NewLiteralStringNode(int nLine, int nCol, const char *pszStr);
extern AstNode *NewLiteralBooleanNode(int nLine, int nCol, bool nBoolean);

// extern froom jExpression.cpp
extern AstNode *NewExpressionNode(int nLine, int nCol, const char *pszOp, AstNode *pLeftNode, AstNode *pRightNode);

// extern froom jCompoundStatement.cpp
extern AstNode *NewCompoundStatementNode(int nLine, int nCol, AstNode *pFirstDeclarationNode, AstNode *pFirstStatementNode);
extern AstNode *NewPrintNode(int nLine, int nCol, AstNode *pExpressionNode);
extern AstNode *NewVariableRefNode(int nLine, int nCol, const char *pszVarName, AstNode *pFirstArrRefNode);
extern AstNode *NewAssignReadNode(int nLine, int nCol, AstNode *pVariableRefNode, AstNode *pExpressionNode);

// extern froom jStatement.cpp
extern AstNode *NewConditionNode(int nLine, int nCol, AstNode *pExpressionNode, AstNode *pThenCompoundStatementNode, AstNode *pElseCompoundStatementNode);
extern AstNode *NewWhileNode(int nLine, int nCol, AstNode *pExpressionNode, AstNode *pCompoundStatementNode);
extern AstNode *NewReturnNode(int nLine, int nCol, AstNode *pExpressionNode);
extern AstNode *NewForNode(int nLine, int nCol, AstNode *pLoopVarNode, AstNode *pAssignSymbolNode, AstNode *pStartIntNode, AstNode *pEndIntNode, AstNode *pCompoundStatementNode);

// extern froom jFunction.cpp
extern AstNode *NewFunctionInvocationNode(int nLine, int nCol, const char *pszFuncName, AstNode *pFirstExpressionNode);
extern AstNode *NewFunctionNode(int nLine, int nCol, const char *pszFuncName, AstNode *pFirstArgNode, AstNode *pReturnTypeNode, AstNode *pFirstStatementNode);

// extern froom SymTab.cpp
extern void SymTab_Init();
extern void SymTab_EnableDump(bool bEnable);

#endif //__JAST_API_H__
