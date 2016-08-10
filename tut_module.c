#include <stdio.h>

#include "tut_module.h"
#include "tut_parser.h"

void Tut_InitModule(TutModule* module, const char* name, const char* code)
{
	module->name = Tut_Strdup(name);
	
	Tut_InitLexer(&module->lexer, code);
	Tut_InitSymbolTable(&module->symbolTable);
	Tut_InitList(&module->exprList);
	
	Tut_ParseModule(module);
}

void Tut_InitModuleFromFile(TutModule* module, const char* filename)
{
	module->name = Tut_Strdup(filename);
	
	FILE* file = fopen(filename, "rb");
	if(!file)
		Tut_ErrorExit("Failed to open file '%s' for reading.\n", filename);
	
	Tut_InitLexerFromFile(&module->lexer, file);
	fclose(file);
	
	module->lexer.context.filename = filename;

	Tut_InitSymbolTable(&module->symbolTable);
	Tut_InitList(&module->exprList);
	
	Tut_ParseModule(module);
}

void Tut_DestroyModule(TutModule* module)
{
	// TODO: Implement this
}
