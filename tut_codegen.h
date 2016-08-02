#ifndef TUT_CODEGEN_H
#define TUT_CODEGEN_H

#include "tut_vm.h"

void Tut_EmitOp(TutVM* vm, uint8_t op);
void Tut_EmitGet(TutVM* vm, TutBool global, int32_t index);
void Tut_EmitSet(TutVM* vm, TutBool global, int32_t index);
void Tut_EmitPushInt(TutVM* vm, int32_t value);
void Tut_EmitPushFloat(TutVM* vm, float value);
void Tut_EmitPushCStr(TutVM* vm, const char* value);
void Tut_EmitCall(TutVM* vm, TutBool ext, int32_t index, uint8_t nargs);
// Returns the bytecode location where the 'pc' is written
int32_t Tut_EmitGoto(TutVM* vm, TutBool cond, int32_t pc);
void Tut_PatchGoto(TutVM* vm, int32_t patchLoc, int32_t pc);
void Tut_EmitFunctionEntryPoint(TutVM* vm);

#endif
