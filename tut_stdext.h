#ifndef TUT_STDEXT_H
#define TUT_STDEXT_H

// Standard external functions (externs) for TutVM

#include "tut_module.h"
#include "tut_vm.h"

TutBool TutStdExt_Printf(TutVM* vm, uint8_t nargs);

void TutStdExt_BindAll(TutModule* module, TutVM* vm);

#endif