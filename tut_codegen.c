#include <string.h>
#include <assert.h>

#include "tut_opcodes.h"
#include "tut_buf.h"
#include "tut_codegen.h"

void Tut_EmitOp(TutVM* vm, uint8_t op)
{
	vm->code[vm->codeSize++] = op;
}

void Tut_EmitMakeVarRef(TutVM* vm, TutBool global, int32_t index)
{
	if (global)
		Tut_EmitOp(vm, TUT_OP_MAKEGLOBALREF);
	else
		Tut_EmitOp(vm, TUT_OP_MAKELOCALREF);
	
	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitMakeDynRef(TutVM * vm, uint16_t offset)
{
	Tut_EmitOp(vm, TUT_OP_MAKEDYNAMICREF);

	Tut_WriteUint16(vm->code, vm->codeSize, offset);
	vm->codeSize += 2;
}

void Tut_EmitMakeFunc(TutVM* vm, TutBool isExtern, int32_t index)
{
	if (!isExtern)
		Tut_EmitOp(vm, TUT_OP_MAKEFUNC);
	else
		Tut_EmitOp(vm, TUT_OP_MAKEEXTERNFUNC);
	
	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitGetRef(TutVM* vm, uint16_t count, uint16_t offset)
{
	if (count == 1)
	{
		Tut_EmitOp(vm, TUT_OP_GETREF1);
	
		Tut_WriteUint16(vm->code, vm->codeSize, offset);
		vm->codeSize += 2;
	}
	else
	{
		Tut_EmitOp(vm, TUT_OP_GETREFN);

		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;

		Tut_WriteUint16(vm->code, vm->codeSize, offset);
		vm->codeSize += 2;
	}
}

void Tut_EmitSetRef(TutVM* vm, uint16_t count, uint16_t offset)
{
	if (count == 1)
	{
		Tut_EmitOp(vm, TUT_OP_SETREF1);

		Tut_WriteUint16(vm->code, vm->codeSize, offset);
		vm->codeSize += 2;
	}
	else
	{
		Tut_EmitOp(vm, TUT_OP_SETREFN);

		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;

		Tut_WriteUint16(vm->code, vm->codeSize, offset);
		vm->codeSize += 2;
	}
}

void Tut_EmitGet(TutVM* vm, TutBool global, int32_t index, uint16_t count)
{
	if (count == 1)
	{
		if (global)
			Tut_EmitOp(vm, TUT_OP_GETGLOBAL1);
		else
			Tut_EmitOp(vm, TUT_OP_GETLOCAL1);
	}
	else
	{
		if (global)
			Tut_EmitOp(vm, TUT_OP_GETGLOBALN);
		else
			Tut_EmitOp(vm, TUT_OP_GETLOCALN);

		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;
	}

	Tut_WriteInt32(vm->code, vm->codeSize, index);
	vm->codeSize += 4;
}

void Tut_EmitSet(TutVM* vm, TutBool global, int32_t index, uint16_t count)
{
	if (count == 1)
	{
		if (global)
			Tut_EmitOp(vm, TUT_OP_SETGLOBAL1);
		else
			Tut_EmitOp(vm, TUT_OP_SETLOCAL1);
	}
	else
	{
		if (global)
			Tut_EmitOp(vm, TUT_OP_SETGLOBALN);
		else
			Tut_EmitOp(vm, TUT_OP_SETLOCALN);

		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;
	}

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

void Tut_EmitPushStr(TutVM* vm, const char* value)
{
	Tut_EmitOp(vm, TUT_OP_PUSH_STR);

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

void Tut_EmitPush(TutVM* vm, uint16_t count)
{
	if (count == 0) return;

	if (count == 1)
		Tut_EmitOp(vm, TUT_OP_PUSH1);
	else
	{
		Tut_EmitOp(vm, TUT_OP_PUSHN);

		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;
	}
}

void Tut_EmitPop(TutVM* vm, uint16_t count)
{
	if (count == 0) return;

	if (count == 1)
		Tut_EmitOp(vm, TUT_OP_POP1);
	else
	{
		Tut_EmitOp(vm, TUT_OP_POPN);
		
		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;
	}
}

void Tut_EmitMove(TutVM* vm, uint16_t numObjects, uint16_t stackSpaces)
{
	if (numObjects == 1)
	{
		Tut_EmitOp(vm, TUT_OP_MOVE1);

		Tut_WriteUint16(vm->code, vm->codeSize, stackSpaces);
		vm->codeSize += 2;
	}
	else
	{
		Tut_EmitOp(vm, TUT_OP_MOVEN);

		Tut_WriteUint16(vm->code, vm->codeSize, numObjects);
		vm->codeSize += 2;

		Tut_WriteUint16(vm->code, vm->codeSize, stackSpaces);
		vm->codeSize += 2;
	}
}

void Tut_EmitCall(TutVM* vm, uint16_t nargs)
{
	Tut_EmitOp(vm, TUT_OP_CALL);

	Tut_WriteUint16(vm->code, vm->codeSize, nargs);
	vm->codeSize += 2;
}

void Tut_EmitRetval(TutVM* vm, uint16_t count)
{
	if (count == 1)
		Tut_EmitOp(vm, TUT_OP_RETVAL1);
	else
	{
		Tut_EmitOp(vm, TUT_OP_RETVALN);
		
		Tut_WriteUint16(vm->code, vm->codeSize, count);
		vm->codeSize += 2;
	}
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