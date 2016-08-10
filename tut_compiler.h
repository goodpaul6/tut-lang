#ifndef TUT_COMPILER_H
#define TUT_COMPILER_H

#include "tut_module.h"
#include "tut_vm.h"

void Tut_CompileModule(TutModule* module, TutVM* vm);
void Tut_BindExternFindIndex(TutModule* module, TutVM* vm, const char* name, TutVMExternFunction fn);

#endif