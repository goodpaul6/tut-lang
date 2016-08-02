#ifndef TUT_COMPILER_H
#define TUT_COMPILER_H

#include "tut_module.h"
#include "tut_vm.h"

typedef struct
{
	const char* name;
	TutVMExternFunction function;
} TutExternDef;

void Tut_CompileModule(TutModule* module, TutVM* vm);
void Tut_BindLibrary(TutModule* module, TutVM* vm, uint32_t numExterns, const TutExternDef* externs);

#endif