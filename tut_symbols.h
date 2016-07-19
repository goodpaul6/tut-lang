#ifndef TUT_SYMBOLS_H
#define TUT_SYMBOLS_H

#include "tut_list.h"

typedef struct TutFuncDecl
{
	struct TutFuncDecl* parent;
	
	int index;
	char* name;

	TutList locals, args;
	TutList nestedFunctions;
} TutFuncDecl;

typedef struct TutVarDecl
{
	TutFuncDecl* parent;
	
	int index, scope;
	char* name;
} TutVarDecl;

typedef struct
{
	TutList functions, globals;
	TutFuncDecl* curFunc;
	int curScope;
	
	int numFunctions, numGlobals;
} TutSymbolTable;

void Tut_InitSymbolTable(TutSymbolTable* table);

TutFuncDecl* Tut_DeclareFunction(TutSymbolTable* table, const char* name);
TutVarDecl* Tut_DeclareArgument(TutSymbolTable* table, const char* name);
TutVarDecl* Tut_DeclareVariable(TutSymbolTable* table, const char* name);

void Tut_PushCurFuncDecl(TutSymbolTable* table, TutFuncDecl* decl);
void Tut_PopCurFuncDecl(TutSymbolTable* table);

TutVarDecl* Tut_GetVarDecl(TutSymbolTable* table, const char* name);
TutFuncDecl* Tut_GetFuncDecl(TutSymbolTable* table, const char* name);

void Tut_DestroySymbolTable(TutSymbolTable* table);

#endif
