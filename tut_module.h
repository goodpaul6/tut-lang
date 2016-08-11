#ifndef TUT_MODULE_H
#define TUT_MODULE_H

#include "tut_lexer.h"
#include "tut_list.h"
#include "tut_symbols.h"

typedef struct TutModule
{
	char* name;

	TutList importedModules;
	TutSymbolTable* symbolTable;
	TutLexer lexer;
	TutList exprList;
} TutModule;

void Tut_InitModule(TutModule* module, TutSymbolTable* table, const char* code);
void Tut_InitModuleFromFile(TutModule* module, TutSymbolTable* table, const char* filename);
void Tut_DestroyModule(TutModule* module);

void Tut_InitModuleCache();
TutModule* Tut_LoadModule(TutSymbolTable* table, const char* filename);
void Tut_DestroyModuleCache();

#endif
