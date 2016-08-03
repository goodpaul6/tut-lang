#ifndef TUT_SYMBOLS_H
#define TUT_SYMBOLS_H

#define TUT_VAR_DECL_INDEX_UNDEFINED -0xFFFF

#include "tut_list.h"
#include "tut_typetag.h"

typedef enum
{
	TUT_FUNC_DECL_NORMAL,
	TUT_FUNC_DECL_EXTERN
} TutFuncDeclType;

typedef struct TutFuncDecl
{
	TutFuncDeclType type;

	TutTypetag* returnType;
	struct TutFuncDecl* parent;
	
	int index;
	char* name;

	TutBool hasVarargs;
	TutList locals, args;
	TutList nestedFunctions;
} TutFuncDecl;

typedef struct TutVarDecl
{
	TutTypetag* typetag;
	TutFuncDecl* parent;
	
	int index, scope;
	char* name;
} TutVarDecl;

typedef struct
{
	TutList usertypes;
	TutList functions, globals;
	TutFuncDecl* curFunc;
	int curScope;
	
	int numFunctions, numExterns;
} TutSymbolTable;

void Tut_InitSymbolTable(TutSymbolTable* table);

TutFuncDecl* Tut_DeclareFunction(TutSymbolTable* table, const char* name);
TutFuncDecl* Tut_DeclareExtern(TutSymbolTable* table, const char* name);
TutVarDecl* Tut_DeclareArgument(TutSymbolTable* table, const char* name, TutTypetag* typetag);
TutVarDecl* Tut_DeclareVariable(TutSymbolTable* table, const char* name, TutTypetag* typetag);

// NOTE: If a typetag by "name" does not exist then it is created
// otherwise, the previously declared typetag is returned
TutTypetag* Tut_DefineType(TutSymbolTable* table, const char* name);

void Tut_PushCurFuncDecl(TutSymbolTable* table, TutFuncDecl* decl);
void Tut_PopCurFuncDecl(TutSymbolTable* table);

// NOTE: If scope is -1, it is automatically set to the symbol table's curScope
TutVarDecl* Tut_GetVarDecl(TutSymbolTable* table, const char* name, int scope);
TutFuncDecl* Tut_GetFuncDecl(TutSymbolTable* table, const char* name);

// NOTE: If a usertype by the name cannot be found, an undefined version of the type
// is created and will automatically be filled when the type is defined
TutTypetag* Tut_RegisterType(TutSymbolTable* table, const char* name);

void Tut_DestroySymbolTable(TutSymbolTable* table);

#endif
