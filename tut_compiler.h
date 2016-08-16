#ifndef TUT_COMPILER_H
#define TUT_COMPILER_H

#include "tut_module.h"
#include "tut_vm.h"

typedef enum
{
	TUT_CFLAG_OPEN_ERROR_GEANY_PATH,
	TUT_CFLAG_OPEN_ERROR_NPP_PATH,
	TUT_CFLAG_COUNT
} Tut_CompilerFlag;

void Tut_SetCompilerFlag(Tut_CompilerFlag flag, const char* value);
void Tut_CompileModule(TutModule* module, TutVM* vm);
void Tut_BindExternFindIndex(TutModule* module, TutVM* vm, const char* name, TutVMExternFunction fn);

#endif