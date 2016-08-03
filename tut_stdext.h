#ifndef TUT_STDEXT_H
#define TUT_STDEXT_H

// Standard external functions (externs) for TutVM

#include "tut_module.h"
#include "tut_vm.h"

uint16_t TutStdExt_Printf(TutVM* vm, const TutObject* args, uint16_t nargs);
uint16_t TutStdExt_Strlen(TutVM* vm, const TutObject* args, uint16_t nargs);

void TutStdExt_BindAll(TutModule* module, TutVM* vm);

#endif