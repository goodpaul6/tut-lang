#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "tut_symbols.h"
#include "tut_util.h"

static TutFuncDecl* MakeFuncDecl(const char* name, TutFuncDeclType type)
{
	TutFuncDecl* decl = Tut_Malloc(sizeof(TutFuncDecl));
	
	decl->type = type;

	decl->returnType = NULL;
	decl->parent = NULL;
	
	decl->hasVarargs = TUT_FALSE;
	decl->index = -1;
	decl->name = Tut_Strdup(name);
	
	Tut_InitList(&decl->locals);
	Tut_InitList(&decl->args);
	Tut_InitList(&decl->nestedFunctions);
	
	return decl;
}

static TutVarDecl* MakeVarDecl(const char* name, TutTypetag* typetag)
{
	TutVarDecl* decl = Tut_Malloc(sizeof(TutVarDecl));
	
	decl->typetag = typetag;
	decl->parent = NULL;
	
	decl->index = -1;
	decl->scope = -1;
	decl->name = Tut_Strdup(name);

	return decl;
}

void Tut_InitSymbolTable(TutSymbolTable* table)
{	
	Tut_InitList(&table->usertypes);
	Tut_InitList(&table->functions);
	Tut_InitList(&table->globals);
	
	table->curFunc = NULL;
	table->curScope = 0;

	table->numFunctions = 0;
	table->numExterns = 0;
	table->numGlobals = 0;
}

TutFuncDecl* Tut_DeclareFunction(TutSymbolTable* table, const char* name)
{
	TutFuncDecl* decl = MakeFuncDecl(name, TUT_FUNC_DECL_NORMAL);
	
	decl->index = table->numFunctions++;
		
	if(table->curFunc)
		Tut_ListAppend(&table->curFunc->nestedFunctions, decl);
	else
		Tut_ListAppend(&table->functions, decl);
	
	return decl;
}

TutFuncDecl* Tut_DeclareExtern(TutSymbolTable* table, const char* name)
{
	TutFuncDecl* decl = MakeFuncDecl(name, TUT_FUNC_DECL_EXTERN);

	decl->index = table->numExterns++;

	if (table->curFunc)
		Tut_ListAppend(&table->curFunc->nestedFunctions, decl);
	else
		Tut_ListAppend(&table->functions, decl);

	return decl;
}

TutVarDecl* Tut_DeclareArgument(TutSymbolTable* table, const char* name, TutTypetag* typetag)
{
	assert(table->curFunc);
	
	TutVarDecl* decl = MakeVarDecl(name, typetag);
	
	decl->parent = table->curFunc;
	decl->index = -((int)table->curFunc->args.length + 1);
	decl->scope = table->curScope;
		
	Tut_ListAppend(&table->curFunc->args, decl);
	
	return decl;
}

TutVarDecl* Tut_DeclareVariable(TutSymbolTable* table, const char* name, TutTypetag* typetag)
{
	TutVarDecl* decl = MakeVarDecl(name, typetag);
	decl->parent = table->curFunc;

	if(table->curFunc)
	{
		decl->index = table->curFunc->locals.length;
		decl->scope = table->curScope;
		
		Tut_ListAppend(&table->curFunc->locals, decl);
	}
	else
	{
		decl->index = table->numGlobals++;
		decl->scope = 0;
		
		Tut_ListAppend(&table->globals, decl);
	}
	
	return decl;
}

TutTypetag* Tut_DefineType(TutSymbolTable* table, const char* name)
{
	TUT_LIST_EACH(node, table->usertypes)
	{
		TutTypetag* tag = node->value;
		assert(tag->user.name);
		
		if(strcmp(tag->user.name, name) == 0)
		{
			tag->user.defined = TUT_TRUE;
			return tag;
		}
	}
	
	TutTypetag* tag = Tut_Malloc(sizeof(TutTypetag));
	
	Tut_InitTypetag(tag, TUT_TYPETAG_USERTYPE);
	tag->user.defined = TUT_TRUE;
	tag->user.name = Tut_Strdup(name);
	
	Tut_ListAppend(&table->usertypes, tag);
	
	return tag;
}

void Tut_PushCurFuncDecl(TutSymbolTable* table, TutFuncDecl* decl)
{
	decl->parent = table->curFunc;
	table->curFunc = decl;
}

void Tut_PopCurFuncDecl(TutSymbolTable* table)
{
	if(table->curFunc)
		table->curFunc = table->curFunc->parent;
}

TutVarDecl* Tut_GetVarDecl(TutSymbolTable* table, const char* name, int scope)
{
	if(scope < 0) scope = table->curScope;
	
	if(table->curFunc)
	{
		TutFuncDecl* funcDecl = table->curFunc;
		
		TUT_LIST_EACH(node, funcDecl->locals)
		{
			TutVarDecl* varDecl = node->value;
			for(int i = scope; i >= 0; --i)
			{
				if(varDecl->scope == i && strcmp(varDecl->name, name) == 0)
					return varDecl;
			}
		}
		
		TUT_LIST_EACH(node, funcDecl->args)
		{
			TutVarDecl* varDecl = node->value;
			if(strcmp(varDecl->name, name) == 0)
				return varDecl;
		}
	}
	
	TUT_LIST_EACH(node, table->globals)
	{
		TutVarDecl* decl = node->value;
		if(strcmp(decl->name, name) == 0)
			return decl;
	}
	
	return NULL;
}

TutFuncDecl* Tut_GetFuncDecl(TutSymbolTable* table, const char* name)
{
	if (table->curFunc)
	{
		TUT_LIST_EACH(node, table->curFunc->nestedFunctions)
		{
			TutFuncDecl* decl = node->value;
			if (strcmp(decl->name, name) == 0)
				return decl;
		}
	}

	TUT_LIST_EACH(node, table->functions)
	{
		TutFuncDecl* decl = node->value;
		if(strcmp(decl->name, name) == 0)
			return decl;
	}
	
	return NULL;
}

TutTypetag* Tut_RegisterType(TutSymbolTable* table, const char* name)
{
	TUT_LIST_EACH(node, table->usertypes)
	{
		TutTypetag* tag = node->value;
		assert(tag->user.name);
		
		if(strcmp(tag->user.name, name) == 0)
			return tag;
	}
	
	TutTypetag* tag = Tut_Malloc(sizeof(TutTypetag));
	
	Tut_InitTypetag(tag, TUT_TYPETAG_USERTYPE);
	tag->user.name = Tut_Strdup(name);
	
	Tut_ListAppend(&table->usertypes, tag);
	
	return tag;
}

void Tut_DestroySymbolTable(TutSymbolTable* table)
{
	// TODO: Write this
}
