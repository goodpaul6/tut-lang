#ifndef TUT_MODULE_H
#define TUT_MODULE_H

#include "tut_lexer.h"
#include "tut_list.h"
#include "tut_symbols.h"

typedef struct
{
	char* name;
	
	TutLexer lexer;
	TutSymbolTable symbolTable;
	TutList exprList;
} TutModule;

void Tut_InitModule(TutModule* module, const char* name, const char* code);
void Tut_InitModuleFromFile(TutModule* module, const char* filename);
void Tut_DestroyModule(TutModule* module);

#endif
