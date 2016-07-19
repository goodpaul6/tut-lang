#include <stddef.h>

#include "tut_symbols.h"
#include "tut_util.h"

static TutFuncDecl* MakeFuncDecl(const char* name)
{
	TutFuncDecl* decl = Tut_Malloc(sizeof(TutFuncDecl));
	
	decl->parent = NULL;
	
	decl->index = -1;
	decl->name = Tut_Strdup(name);
	
	Tut_InitList(&decl->locals);
	Tut_InitList(&decl->args);
	Tut_InitList(&decl->nestedFunctions);
	
	return decl;
}

static TutVarDecl* MakeVarDecl(const char* name)
{
	TutVarDecl* decl = Tut_Malloc(sizeof(TutVarDecl));
	
	decl->parent = NULL;
	
	decl->index = -1;
	decl->scope = -1;
	decl->name = Tut_Strdup(name);

	return decl;
}

void Tut_InitSymbolTable(TutSymbolTable* table)
{	
	Tut_InitList(&table->functions);
	Tut_InitList(&table->globals);
	
	table->curFunc = NULL;
	table->curScope = 0;

	table->numFunctions = 0;
	table->numGlobals = 0;
}

TutFuncDecl* Tut_DeclareFunction(TutSymbolTable* table, const char* name)
{
	TutFuncDecl* decl = MakeFuncDecl(name);
	
	decl->index = table->numFunctions++;
		
	if(table->curFunc)
		Tut_ListAppend(&table->curFunc->nestedFunctions, decl);
	else
		Tut_ListAppend(&table->functions);
	
	return decl;
}

TutVarDecl* Tut_DeclareVariable(TutSymbolTable* table, const char* name)
{
	TutVarDecl* decl = MakeVarDecl(name);
	
	if(table->curFunc)
	{
		decl->index = table->curFunc->locals.length;
		decl->scope = table->curScope;
		
		Tut_ListAppend(&table->curFunc->locals, decl);
	}
	else
	{
		
	}
}

void Tut_PushCurFuncDecl(TutSymbolTable* table, TutFuncDecl* decl);
void Tut_PopCurFuncDecl(TutSymbolTable* table);

TutVarDecl* Tut_GetVarDecl(TutSymbolTable* table, const char* name);
TutFuncDecl* Tut_GetFuncDecl(TutSymbolTable* table, const char* name);

void Tut_DestroySymbolTable(TutSymbolTable* table);
