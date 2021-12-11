#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JAST/jast_internal.h"

#define MAX_STACK_LEVELS	128
#define MAX_TOTAL_SYMBOLS	65536

// Default not to dump the whole symbol table before pop.
bool g_bDumpOnPop = false;

// Initialize the symbol table stack level with -1 and g_pnStackIndex[0] = 0 before adding anything.
int g_nStackLevel = -1;		
int g_pnStackIndex[MAX_STACK_LEVELS] = {0};
struct Symbol g_oSymTab[MAX_TOTAL_SYMBOLS];

int  SymTab_GetLevel(int n) { return g_oSymTab[n].nLevel; }
const char *SymTab_GetName(int n) { return g_oSymTab[n].pszName; }
const char *SymTab_GetKind(int n) { return g_oSymTab[n].pszKind; }
const char *SymTab_GetScalerType(int n) { return g_oSymTab[n].pszScalerType; }
const char *SymTab_GetTypeStr(int n) { return g_oSymTab[n].pszTypeStr; }
const char *SymTab_GetAttr(int n) { return g_oSymTab[n].pszAttr; }
SymbolValue_t SymTab_GetKindValue(int n) { return g_oSymTab[n].nSymKind; }
SymbolValue_t SymTab_GetTypeValue(int n) { return g_oSymTab[n].nSymType; }
AstNode *SymTab_GetAstNode(int n) { return g_oSymTab[n].pAst; }

void SymTab_EnableDump(bool bEnable) { g_bDumpOnPop = bEnable; }
int  SymTab_GetCurrStackLevel() {return g_nStackLevel; }

// Initialize the symbol table.
void SymTab_Init()
{
	g_nStackLevel = -1;
	g_pnStackIndex[0] = 0;
}

// Release allocated string on releasing the symbol table.
void SymTab_Release()
{
	int i;

	for(i = g_pnStackIndex[g_nStackLevel + 1]; i >= 0; i--){
		free(g_oSymTab[i].pszName);
		free(g_oSymTab[i].pszKind);
		free(g_oSymTab[i].pszScalerType);
		free(g_oSymTab[i].pszTypeStr);
		free(g_oSymTab[i].pszAttr);
	}
}

// Dump the whole symbol table from stack bottom to top.
void SymTab_Dump() 
{
	int i;
	struct Symbol *s;

	for (i = 0; i < 110; ++i) printf("=");
	printf("\n");
	printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type", "Attribute");
	for (i = 0; i < 110; ++i) printf("-");
	printf("\n");
	for(i = 0; i < g_pnStackIndex[g_nStackLevel + 1]; i++){
		s = &g_oSymTab[i];
		printf("%-33s%-11s%d%-10s%-17s%-11s\n", s->pszName, s->pszKind, s->nLevel, (s->nLevel == 0) ? "(global)" : "(local)", s->pszTypeStr, s->pszAttr);
	}
	for (i = 0; i < 110; ++i) printf("-");
	printf("\n");
}

// Push a symbol table to the stack by increasing stack level and init the table start and end indexes.
int SymTab_Push()
{
	g_nStackLevel++;
	g_pnStackIndex[g_nStackLevel + 1] = g_pnStackIndex[g_nStackLevel];
	return g_nStackLevel;
}

// Remove the symbol table on stack top by decreasing the stack level.
int SymTab_Pop()
{
	if (g_bDumpOnPop)
		SymTab_Dump();

	g_nStackLevel--;
	return g_nStackLevel;
}

// Insert the input symbol on the very top of the symbol table stack.
int SymTab_Insert(const char *pszName, const char *pszKind, const char *pszScalerType, const char *pszTypeStr, const char *pszAttr, AstNode *pAst)
{
	int n;
	struct Symbol *s;

	n = g_pnStackIndex[g_nStackLevel + 1];
	s = &g_oSymTab[n];
	s->nLevel = g_nStackLevel;
	s->pszName = strdup(pszName);
	s->pszKind = strdup(pszKind);
	s->pszScalerType = strdup(pszScalerType);
	s->pszTypeStr = strdup(pszTypeStr);
	s->pszAttr = strdup(pszAttr);
	s->nSymKind = GetSymbolValue(pszKind);
	s->nSymType = GetSymbolValue(pszScalerType);
	s->pAst = pAst;
	g_pnStackIndex[g_nStackLevel + 1]++;
	return n;
}

// Get the table index of the input symbol name, linearly searching from stack top to bottom.
int SymTab_Lookup(const char *pszName)
{
	int i;

	for(i = g_pnStackIndex[g_nStackLevel + 1] - 1; i >= 0; i--){
		if (strcmp(g_oSymTab[i].pszName, pszName) == 0)
			return i;
	}
	return -1;
}


