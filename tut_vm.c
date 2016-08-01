#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "tut_vm.h"
#include "tut_objects.h"
#include "tut_buf.h"
#include "tut_opcodes.h"

void Tut_InitVM(TutVM* vm)
{
	Tut_InitArray(&vm->integers, sizeof(int32_t));
	Tut_InitArray(&vm->floats, sizeof(float));
	Tut_InitArray(&vm->strings, sizeof(const char*));
	Tut_InitArray(&vm->functionPcs, sizeof(int32_t));

	Tut_InitArray(&vm->returnFrames, sizeof(TutReturnFrame));

	vm->codeSize = 0;

	vm->sp = 0;
	vm->pc = -1;
	vm->fp = 0;
}

void Tut_Push(TutVM* vm, const TutObject* object)
{
	if(vm->sp >= TUT_VM_STACK_SIZE)
	{
		fprintf(stderr, "VM Stack Overflow!\n");
		vm->pc = -1;
	}
	
	memcpy(&vm->stack[vm->sp], object, sizeof(*object));
	vm->sp += 1;
}

void Tut_Pop(TutVM* vm, TutObject* object)
{
	if(vm->sp <= 0)
	{
		fprintf(stderr, "VM Stack Underflow!\n");
		vm->pc = -1;
	}

	vm->sp -= 1;
	memcpy(object, &vm->stack[vm->sp], sizeof(*object));
}

void Tut_PushBool(TutVM* vm, TutBool value)
{
	TutObject object;
	object.bv = value;

	Tut_Push(vm, &object);
}

void Tut_PushInt(TutVM* vm, int32_t value)
{
	TutObject object;
	object.iv = value;

	Tut_Push(vm, &object);
}

void Tut_PushFloat(TutVM* vm, float value)
{
	TutObject object;
	object.fv = value;

	Tut_Push(vm, &object);
}

void Tut_PushCString(TutVM* vm, const char* string)
{
	TutObject object;
	object.csv = string;

	Tut_Push(vm, &object);
}

void Tut_PushString(TutVM* vm, const char* string)
{
	TutObject object;
	object.sv = Tut_Strdup(string);

	Tut_Push(vm, &object);
}

TutBool Tut_PopBool(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return (TutBool)object.bv;
}

int32_t Tut_PopInt(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return object.iv;
}

float Tut_PopFloat(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return object.fv;
}

const char* Tut_PopCString(TutVM* vm)
{

	TutObject object;
	Tut_Pop(vm, &object);

	return object.csv;
}

char* TutPopString(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return object.sv;
}

void Tut_ExecuteCycle(TutVM* vm)
{
	if(vm->pc < 0) return;
	
	uint8_t op = vm->code[vm->pc++];
	
	switch(op)
	{
		case TUT_OP_PUSH_TRUE:
		{
			TutBool value = (TutBool)vm->code[vm->pc++];
			Tut_PushBool(vm, TUT_TRUE);
		} break;
		
		case TUT_OP_PUSH_FALSE:
		{
			TutBool value = (TutBool)vm->code[vm->pc++];
			Tut_PushBool(vm, TUT_FALSE);
		} break;

		case TUT_OP_PUSH_INT:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			
			int32_t value = TUT_ARRAY_GET_VALUE(&vm->integers, index, int32_t);
			Tut_PushInt(vm, value);
		} break;
		
		case TUT_OP_PUSH_FLOAT:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			
			float value = TUT_ARRAY_GET_VALUE(&vm->floats, index, float);
			Tut_PushFloat(vm, value);
		} break;
		
		case TUT_OP_PUSH_CSTR:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			
			const char* data = TUT_ARRAY_GET_VALUE(&vm->strings, index, const char*);
			Tut_PushCString(vm, data);
		} break;

		case TUT_OP_POP:
		{
			if (vm->sp <= 0)
			{
				fprintf(stderr, "VM Stack Underflow (TUT_OP_POP)!\n");
				vm->pc = -1;
				return;
			}
			--vm->sp;
		} break;
		
		case TUT_OP_GET_GLOBAL:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_Push(vm, &vm->globals[index]);
		} break;

		case TUT_OP_SET_GLOBAL:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_Pop(vm, &vm->globals[index]);
		} break;

		case TUT_OP_GET_LOCAL:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_Push(vm, &vm->stack[vm->fp + index]);
		} break;

		case TUT_OP_SET_LOCAL:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			
			Tut_Pop(vm, &vm->stack[vm->fp + index]);
		} break;

#define BIN_OP_INT(name, op) 
#define BIN_OP_FLOAT(name, op) case name: { float b = Tut_PopFloat(vm); float a = Tut_PopFloat(vm); Tut_PushFloat(vm, a op b); } break;
		case TUT_OP_ADDI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a + b); 
		} break;
		
		case TUT_OP_SUBI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a - b); 
		} break;

		case TUT_OP_MULI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a * b); 
		} break;
		
		case TUT_OP_DIVI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a / b); 
		} break;

		BIN_OP_INT(TUT_OP_ADDI, +)
		BIN_OP_INT(TUT_OP_SUBI, -)
		BIN_OP_INT(TUT_OP_MULI, *)
		BIN_OP_INT(TUT_OP_DIVI, /)
		
		BIN_OP_FLOAT(TUT_OP_ADDF, +)
		BIN_OP_FLOAT(TUT_OP_SUBF, -)
		BIN_OP_FLOAT(TUT_OP_MULF, *)
		BIN_OP_FLOAT(TUT_OP_DIVF, /)	

#undef BIN_OP_INT
#undef BIN_OP_FLOAT

		case TUT_OP_CALL:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			uint8_t nargs = vm->code[vm->pc++];
			
			TutReturnFrame frame;

			frame.nargs = nargs;
			frame.pc = vm->pc;
			frame.fp = vm->fp;

			Tut_ArrayPush(&vm->returnFrames, &frame);

			vm->pc = TUT_ARRAY_GET_VALUE(&vm->functionPcs, index, int32_t);
			vm->fp = vm->sp;
		} break;

		case TUT_OP_RET:
		{
			if (vm->returnFrames.length <= 0)
			{
				vm->pc = -1;
				return;
			}

			TutReturnFrame frame;
			Tut_ArrayPop(&vm->returnFrames, &frame);

			vm->sp = frame.fp;
			vm->sp -= frame.nargs;
			vm->pc = frame.pc;
		} break;

		case TUT_OP_RETVAL:
		{
			TutObject object;
			Tut_Pop(vm, &object);

			if (vm->returnFrames.length <= 0)
			{
				vm->pc = -1;
				return;
			}

			TutReturnFrame frame;
			Tut_ArrayPop(&vm->returnFrames, &frame);

			vm->sp = vm->fp;
			vm->sp -= frame.nargs;
			vm->fp = frame.fp;
			vm->pc = frame.pc;

			Tut_Push(vm, &object);
		} break;

		case TUT_OP_GOTO:
		{
			int32_t pc = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc = pc;
		} break;

		case TUT_OP_GOTOFALSE:
		{
			int32_t pc = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			
			TutBool value = Tut_PopBool(vm);
			if(!value)
				vm->pc = pc;
		} break;

		case TUT_OP_HALT:
		{
			vm->pc = -1;
		} break;
	}
}

void Tut_DestroyVM(TutVM* vm)
{
	// TODO: Implement this
}
