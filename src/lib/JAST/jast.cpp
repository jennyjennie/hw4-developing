#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "JAST/jast.h"
#include "JAST/jast_internal.h"

// extern from scanner.l
extern char *LexGetSourceCode(int nLine);

const SymbolValue_t k_pnSymbolValues[] = { 
	kProgram, kFunction, kParameter, kVariable, kLoopVar, kConstant, kInteger, kReal, kBoolean, kString, kVoid, 
	kADD, kMINUS, kMULTIPLY, kDIVIDE, kNEG, kMOD, kAND, kOR, kNOT, kLT, kLE, kEQ, kGE, kGT, kNE, 
	kUnknown 
};
const char *k_ppszSymbols[] = { 
	"program", "function", "parameter", "variable", "loop_var", "constant", "integer", "real", "boolean", "string", "void", 
	"+", "-", "*", "/", "neg", "mod", "and", "or", "not",  "<", "<=", "=", ">=", ">", "<>", 
	NULL 
};

// ----------------------------------------------------------------
// Some utility functions and symbol-value lookup.
// ----------------------------------------------------------------

// Search a list of strings, with NULL at end of the list, and return the index for the input symbol,
// or the index of NULL end if the item is not found in the list.
int SearchStringItem(const char *pszItem, const char *ppszItems[])
{
	int i;

	for(i = 0; ppszItems[i] != NULL; i++){
		if (strcmp(pszItem, ppszItems[i]) == 0)
			break;
	}
	return i;
}

// Search a list of symbol strings, with NULL at end of the list, and return the corresponsing value for the input symbol.
SymbolValue_t GetSymbolValue(const char *pszSymbol)
{
	int n = SearchStringItem(pszSymbol, k_ppszSymbols);
	return k_pnSymbolValues[n];
}

// Get symbol string corresponding to the input symbol value. 
// Assume k_pnSymbolValues[] is {0, 1, 2, 3, ......, -1}.
const char *GetSymbolString(SymbolValue_t n)
{
	return k_ppszSymbols[n];
}

// Check if the input symbol value existing in a list of symbol values, with kUnknown at end of the list.
bool InSymbolValueSet(SymbolValue_t nSymbol, const SymbolValue_t pnSymbols[])
{
	int i;

	for(i = 0; pnSymbols[i] != kUnknown; i++){
		if (nSymbol == pnSymbols[i])
			return true;
	}
	return false;
}

void ErrorMessage(AstNode *pAst, const char *format, ...)
{
	va_list args;

	fprintf(stderr, "<Error> Found in line %d, column %d: ", pAst->location.line, pAst->location.col);
	
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, "%s\n", LexGetSourceCode(pAst->location.line - 1));
	for (int i = 0; i < pAst->location.col - 1; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "^\n");
}

// Get array type string when array undersubscript in error messages
const char *GetArrayTypeString(AstNode *pAst)
{	
	int n;
	const char *pszVarName, *pszArrayTypeStr;
	ExpressionNode *pNode = (ExpressionNode *)pAst->pBody;

	// Get the variable name in the expression node.
	pszVarName = ((VariableRefNode *)pNode->pLeftNode->pBody)->pszVarName;
	
	// Look up in the symbol table and get the array type string.
	n = SymTab_Lookup(pszVarName);
	pszArrayTypeStr = SymTab_GetTypeStr(n);
	
	return pszArrayTypeStr;
}

// ----------------------------------------------------------------
// Print, semantic analysis, and code generation functions
// ----------------------------------------------------------------
void PrintLeadingTabs(int nTab)
{
	for(int i = 0; i < nTab; i++)
		printf("  ");
}

int PrintAstNode(AstNode *pAst, int nLevel)
{
	if (pAst && pAst->print)
		return pAst->print(pAst, nLevel);
	else
		return 0;
}

int PrintAstList(AstNode *pFirstAst, int nLevel)
{
	AstNode *p;

	p = pFirstAst;
	while(p){
		PrintAstNode(p, nLevel);
		p = p->pNext;
	}	
	return 0;
}

int VisitAstNode(AstNode *pAst)
{
	if (pAst && pAst->visit)
		return pAst->visit(pAst);
	else
		return 0;
}

int VisitAstList(AstNode *pFirstAst, bool bErrorBreak)
{
	int nErr;
	AstNode *p;

	nErr = 0;
	p = pFirstAst;
	while(p){
		nErr += VisitAstNode(p);
		if (nErr > 0 && bErrorBreak)
			break;
		p = p->pNext;
	}	
	return nErr;
}

int CodeGenAstNode(AstNode *pAst)
{
	if (pAst && pAst->codegen)
		return pAst->codegen(pAst);
	else
		return 0;
}

int CodeGenAstList(AstNode *pFirstAst)
{
	AstNode *p;

	p = pFirstAst;
	while(p){
		CodeGenAstNode(p);
		p = p->pNext;
	}	
	return 0;
}

// ----------------------------------------------------------------
// Tree construction functions 
// ----------------------------------------------------------------
AstNode *NewAstNode(int nLine, int nCol, void *pBody, int (*funcPrint)(AstNode*, int), int (*funcVisit)(AstNode*), int (*funcCodeGen)(AstNode*))
{
	AstNode *node = new AstNode;

	node->pBody = pBody;
	node->print = funcPrint;
	node->visit = funcVisit;
	node->codegen = funcCodeGen;
	node->pNext = NULL;
	node->location.line = nLine;
	node->location.col = nCol;
	return node;
}

AstNode *DupAstNode(AstNode *pAst)
{
	AstNode *node = new AstNode;
	memcpy(node, pAst, sizeof(AstNode));
	return node;
}

AstNode *AddSiblingNode(AstNode *pNode, AstNode *pSibling)
{
	AstNode *p;

	p = pNode;
	while(p->pNext)
		p = p->pNext;
	p->pNext = pSibling;
	return pNode;
}

int AstLinkLength(AstNode *pFirstNode)
{
	int n;
	AstNode *p;

	n = 0;
	p = pFirstNode;
	while(p){
		n++;
		p = p->pNext;
	}
	return n;
}

