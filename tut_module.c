#include <stdio.h>
#include <string.h>

#include "tut_module.h"
#include "tut_parser.h"

static TutList ModuleCache;

void Tut_InitModule(TutModule* module, TutSymbolTable* table, const char* code)
{
	module->name = NULL;
	module->symbolTable = table;

	Tut_InitList(&module->importedModules);
	Tut_InitLexer(&module->lexer, code);
	Tut_InitList(&module->exprList);
	
	Tut_ParseModule(module);
}

void Tut_InitModuleFromFile(TutModule* module, TutSymbolTable* table, const char* filename)
{
	module->name = NULL;
	module->symbolTable = table;
	
	FILE* file = fopen(filename, "rb");
	if(!file)
		Tut_ErrorExit("Failed to open file '%s' for reading.\n", filename);
	
	Tut_InitLexerFromFile(&module->lexer, file);
	fclose(file);
	
	module->lexer.context.filename = filename;

	Tut_InitList(&module->importedModules);
	Tut_InitList(&module->exprList);
	
	Tut_ParseModule(module);
}

void Tut_DestroyModule(TutModule* module)
{
	// TODO: Implement this
}

void Tut_InitModuleCache()
{
	Tut_InitList(&ModuleCache);
}

TutModule* Tut_LoadModule(TutSymbolTable* table, const char* filename)
{
	TUT_LIST_EACH(node, ModuleCache)
	{
		TutModule* mod = node->value;
		if (mod->lexer.context.filename &&
			strcmp(mod->lexer.context.filename, filename) == 0)
			return mod;
	}

	TutModule* module = Tut_Malloc(sizeof(TutModule));

	Tut_InitModuleFromFile(module, table, filename);
	Tut_ListAppend(&ModuleCache, module);

	return module;
}

void Tut_ClearModuleCache()
{
	TUT_LIST_EACH(node, ModuleCache)
		Tut_DestroyModule(node->value);
	
	ModuleCache.head = ModuleCache.tail = NULL;
	ModuleCache.length = 0;
}

void Tut_DestroyModuleCache()
{
	TUT_LIST_EACH(node, ModuleCache)
		Tut_DestroyModule(node->value);
	Tut_DestroyList(&ModuleCache);
}