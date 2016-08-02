#include <string.h>
#include <assert.h>

#include "tut_opcodes.h"
#include "tut_buf.h"
#include "tut_codegen.h"

void Tut_EmitOp(TutVM* vm, uint8_t op)
{
	vm->code[vm->codeSize++] = op;
}

void Tut_EmitGet(TutVM* vm, TutBool global, int32_t index)
{
	if (global)
		Tut_EmitOp(vm, TUT_OP_GET_GLOBAL);
	else
		Tut_EmitOp(vm, TUT_OP_GET_LOCAL);
	
	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitSet(TutVM* vm, TutBool global, int32_t index)
{
	if (global)
		Tut_EmitOp(vm, TUT_OP_SET_GLOBAL);
	else
		Tut_EmitOp(vm, TUT_OP_SET_LOCAL);

	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitPushInt(TutVM* vm, int32_t value)
{
	Tut_EmitOp(vm, TUT_OP_PUSH_INT);

	int32_t index = -1;

	for (int i = 0; i < vm->integers.length; ++i)
	{
		if (TUT_ARRAY_GET_VALUE(&vm->integers, i, int) == value)
		{
			index = i;
			break;
		}
	}

	if (index < 0)
	{
		index = vm->integers.length;
		Tut_ArrayPush(&vm->integers, &value);
	}

	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitPushFloat(TutVM* vm, float value)
{
	Tut_EmitOp(vm, TUT_OP_PUSH_FLOAT);

	int32_t index = -1;

	for (int i = 0; i < vm->floats.length; ++i)
	{
		if (TUT_ARRAY_GET_VALUE(&vm->floats, i, float) == value)
		{
			index = i;
			break;
		}
	}

	if (index < 0)
	{
		index = vm->floats.length;
		Tut_ArrayPush(&vm->floats, &value);
	}

	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitPushCStr(TutVM* vm, const char* value)
{
	Tut_EmitOp(vm, TUT_OP_PUSH_CSTR);

	int32_t index = -1;

	for (int i = 0; i < vm->strings.length; ++i)
	{
		if (strcmp(TUT_ARRAY_GET_VALUE(&vm->strings, i, const char*), value) == 0)
		{
			index = i;
			break;
		}
	}

	if (index < 0)
	{
		const char* str = Tut_Strdup(value);
		
		index = vm->strings.length;
		Tut_ArrayPush(&vm->strings, &str);
	}

	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitCall(TutVM* vm, TutBool ext, int32_t index, uint8_t nargs)
{
	if (!ext)
		Tut_EmitOp(vm, TUT_OP_CALL);
	else
		Tut_EmitOp(vm, TUT_OP_CALL_EXTERN);

	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;

	vm->code[vm->codeSize++] = nargs;
}

int32_t Tut_EmitGoto(TutVM* vm, TutBool cond, int32_t pc)
{
	if (!cond)
		Tut_EmitOp(vm, TUT_OP_GOTO);
	else
		Tut_EmitOp(vm, TUT_OP_GOTOFALSE);

	Tut_WriteInt32(vm->code, vm->codeSize, pc);
	vm->codeSize += 4;

	return vm->codeSize - 4;
}

void Tut_PatchGoto(TutVM* vm, int32_t patchLoc, int32_t pc)
{
	Tut_WriteInt32(vm->code, patchLoc, pc);
}

void Tut_EmitFunctionEntryPoint(TutVM* vm)
{
	Tut_ArrayPush(&vm->functionPcs, &vm->codeSize);
}