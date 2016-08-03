#ifndef TUT_CODEGEN_H
#define TUT_CODEGEN_H

#include "tut_vm.h"

void Tut_EmitOp(TutVM* vm, uint8_t op);
void Tut_EmitGet(TutVM* vm, TutBool global, int32_t index, uint16_t count);
void Tut_EmitSet(TutVM* vm, TutBool global, int32_t index, uint16_t count);
void Tut_EmitPushInt(TutVM* vm, int32_t value);
void Tut_EmitPushFloat(TutVM* vm, float value);
void Tut_EmitPushCStr(TutVM* vm, const char* value);
void Tut_EmitPush(TutVM* vm, uint16_t count);
void Tut_EmitPop(TutVM* vm, uint16_t count);
void Tut_EmitMove(TutVM* vm, uint16_t numObjects, uint16_t stackSpaces);
void Tut_EmitCall(TutVM* vm, TutBool ext, int32_t index, uint16_t nargs);
void Tut_EmitRetval(TutVM* vm, uint16_t count);
// Returns the bytecode location where the 'pc' is written
int32_t Tut_EmitGoto(TutVM* vm, TutBool cond, int32_t pc);
void Tut_PatchGoto(TutVM* vm, int32_t patchLoc, int32_t pc);
void Tut_EmitFunctionEntryPoint(TutVM* vm);

#endif
