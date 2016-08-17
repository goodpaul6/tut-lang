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
	
	Tut_InitArray(&vm->externNames, sizeof(const char*));
	Tut_InitArray(&vm->externs, sizeof(TutVMExternFunction));

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

void Tut_PushString(TutVM* vm, const char* string)
{
	TutObject object;
	object.sv = Tut_Strdup(string);

	Tut_Push(vm, &object);
}

void Tut_PushStringNoCopy(TutVM* vm, const char* string)
{
	TutObject object;
	object.sv = string;

	Tut_Push(vm, &object);
}

void Tut_PushRef(TutVM* vm, void* ref)
{
	TutObject object;
	object.ref = ref;

	Tut_Push(vm, &object);
}

void Tut_PushFunc(TutVM* vm, TutBool isExtern, int32_t index)
{
	TutObject object;

	object.func.isExtern = isExtern;
	object.func.index = index;

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

const char* Tut_PopString(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return object.sv;
}

void* Tut_PopRef(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return object.ref;
}

TutFunctionObject Tut_PopFunc(TutVM* vm)
{
	TutObject object;
	Tut_Pop(vm, &object);

	return object.func;
}

void Tut_BindExtern(TutVM* vm, uint32_t index, const char* name, TutVMExternFunction ext)
{
	assert(index < vm->externNames.length &&
		   index < vm->externs.length);
	
	const char* str = Tut_Strdup(name);

	Tut_ArraySet(&vm->externNames, index, &str);
	Tut_ArraySet(&vm->externs, index, &ext);
}

#if 1
#define DEBUG_CYCLE(op, format, ...) if(debugFlags & TUT_VM_DEBUG_OP) printf("%s " format "\n", #op, __VA_ARGS__)
#else
#define DEBUG_CYCLE(op, format, ...)
#endif

void Tut_ExecuteCycle(TutVM* vm, int debugFlags)
{
	if(vm->pc < 0) return;
	
	uint8_t op = vm->code[vm->pc++];

	if (debugFlags & TUT_VM_DEBUG_REGS) printf("sp=%d, fp=%d\n", vm->sp, vm->fp);
	if(debugFlags & TUT_VM_DEBUG_OP) printf("%d: ", vm->pc);

	switch (op)
	{
		case TUT_OP_PUSH_TRUE:
		{
			DEBUG_CYCLE(TUT_OP_PUSH_TRUE, "");
			Tut_PushBool(vm, TUT_TRUE);
		} break;

		case TUT_OP_PUSH_FALSE:
		{
			DEBUG_CYCLE(TUT_OP_PUSH_FALSE, "");
			Tut_PushBool(vm, TUT_FALSE);
		} break;

		case TUT_OP_PUSH_INT:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			int32_t value = TUT_ARRAY_GET_VALUE(&vm->integers, index, int32_t);
			Tut_PushInt(vm, value);

			DEBUG_CYCLE(TUT_OP_PUSH_INT, "%d", value);
		} break;

		case TUT_OP_PUSH_FLOAT:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			float value = TUT_ARRAY_GET_VALUE(&vm->floats, index, float);
			Tut_PushFloat(vm, value);

			DEBUG_CYCLE(TUT_OP_PUSH_FLOAT, "%f", value);
		} break;

		case TUT_OP_PUSH_STR:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			const char* data = TUT_ARRAY_GET_VALUE(&vm->strings, index, const char*);
			Tut_PushStringNoCopy(vm, data);

			DEBUG_CYCLE(TUT_OP_PUSH_STR, "%s", data);
		} break;

		case TUT_OP_PUSH_NULL:
		{
			Tut_PushRef(vm, NULL);

			DEBUG_CYCLE(TUT_OP_PUSH_NULL, "");
		} break;

		case TUT_OP_MAKEGLOBALREF:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_PushRef(vm, &vm->globals[index]);

			DEBUG_CYCLE(TUT_OP_MAKEGLOBALREF, "%d (%x)", index, (uintptr_t)(&vm->globals[index]));
		} break;

		case TUT_OP_MAKELOCALREF:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_PushRef(vm, &vm->stack[vm->fp + index]);

			DEBUG_CYCLE(TUT_OP_MAKELOCALREF, "%d (%x)", index, (uintptr_t)(&vm->stack[vm->fp + index]));
		} break;

		case TUT_OP_MAKEDYNAMICREF:
		{
			uint16_t offset = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			TutObject* ref = Tut_PopRef(vm);
			Tut_PushRef(vm, &ref[offset]);

			DEBUG_CYCLE(TUT_OP_MAKEDYNAMICREF, "%d (%x)", offset, (uintptr_t)(&ref[offset]));
		} break;

		case TUT_OP_MAKEFUNC:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_PushFunc(vm, TUT_FALSE, index);

			DEBUG_CYCLE(TUT_OP_MAKEFUNC, "%d", index);
		} break;

		case TUT_OP_MAKEEXTERNFUNC:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
	
			Tut_PushFunc(vm, TUT_TRUE, index);

			DEBUG_CYCLE(TUT_OP_MAKEFUNC, "%s(%d)", TUT_ARRAY_GET_VALUE(&vm->externNames, index, const char*), index);
		} break;

		case TUT_OP_PUSHN:
		{
			uint16_t n = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			if (vm->sp + n > TUT_VM_STACK_SIZE)
			{
				fprintf(stderr, "VM Stack Overflow (TUT_OP_PUSHN)!\n");
				vm->pc = -1;
			}
			vm->sp += n;

			DEBUG_CYCLE(TUT_OP_PUSHN, "%d", n);
		} break;

		case TUT_OP_PUSH1:
		{
			if (vm->sp >= TUT_VM_STACK_SIZE)
			{
				fprintf(stderr, "VM Stack Overflow (TUT_OP_PUSH1)!\n");
				vm->pc = -1;
			}
			++vm->sp;

			DEBUG_CYCLE(TUT_OP_PUSH1, "");
		} break;

		case TUT_OP_POPN:
		{
			uint16_t n = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			if (vm->sp - n < 0)
			{
				fprintf(stderr, "VM Stack Underflow (TUT_OP_POPN)!\n");
				vm->pc = -1;
				return;
			}
			vm->sp -= n;

			DEBUG_CYCLE(TUT_OP_POPN, "%d", n);
		} break;

		case TUT_OP_POP1:
		{
			if (vm->sp <= 0)
			{
				fprintf(stderr, "VM Stack Underflow (TUT_OP_POP1)!\n");
				vm->pc = -1;
				return;
			}
			--vm->sp;

			DEBUG_CYCLE(TUT_OP_POP1, "");
		} break;

		case TUT_OP_MOVEN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;
			uint16_t stackSpaces = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int32_t targetSp = vm->sp - numObjects - stackSpaces;

			if (targetSp < 0)
			{
				fprintf(stderr, "VM Stack Underflow (TUT_OP_MOVEN)!\n");
				vm->pc = -1;
				return;
			}
			
			memmove(&vm->stack[targetSp], &vm->stack[vm->sp - numObjects], sizeof(TutObject) * numObjects);
			vm->sp = targetSp + numObjects;

			DEBUG_CYCLE(TUT_OP_MOVEN, "%d, %d", numObjects, stackSpaces);
		} break;
		
		case TUT_OP_MOVE1:
		{
			// Copied from TUT_OP_MOVEN
			uint16_t numObjects = 1;
			uint16_t stackSpaces = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int32_t targetSp = vm->sp - numObjects - stackSpaces;

			if (targetSp < 0)
			{
				fprintf(stderr, "VM Stack Underflow (TUT_OP_MOVEN)!\n");
				vm->pc = -1;
				return;
			}

			memmove(&vm->stack[targetSp], &vm->stack[vm->sp - numObjects], sizeof(TutObject) * numObjects);
			vm->sp = targetSp + numObjects;

			DEBUG_CYCLE(TUT_OP_MOVE1, "%d", stackSpaces);
		} break;

		case TUT_OP_GETGLOBALN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			memcpy(&vm->stack[vm->sp], &vm->globals[index], sizeof(TutObject) * numObjects);
			vm->sp += numObjects;

			DEBUG_CYCLE(TUT_OP_GETGLOBALN, "%d, %d", numObjects, index);
		} break;

		case TUT_OP_GETGLOBAL1:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_Push(vm, &vm->globals[index]);
			DEBUG_CYCLE(TUT_OP_GETGLOBAL1, "%d", index);
		} break;
		 
		case TUT_OP_SETGLOBALN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			if (vm->sp - numObjects < 0)
			{
				fprintf(stderr, "Error: Stack Underflow! (TUT_OP_SETGLOBALN)\n");
				vm->pc = -1;
				return;
			}

			memcpy(&vm->globals[index], &vm->stack[vm->sp - numObjects], sizeof(TutObject) * numObjects);
			vm->sp -= numObjects;

			DEBUG_CYCLE(TUT_OP_SETGLOBALN, "%d, %d", numObjects, index);
		} break;

		case TUT_OP_SETGLOBAL1:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			if (vm->sp <= 0)
			{
				fprintf(stderr, "Error: Stack Underflow! (TUT_OP_SETGLOBAL1)\n");
				vm->pc = -1;
				return;
			}

			Tut_Pop(vm, &vm->globals[index]);

			DEBUG_CYCLE(TUT_OP_SETGLOBAL1, "%d", index);
		} break;

		case TUT_OP_GETLOCALN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			memcpy(&vm->stack[vm->sp], &vm->stack[vm->fp + index], sizeof(TutObject) * numObjects);
			vm->sp += numObjects;

			DEBUG_CYCLE(TUT_OP_GETLOCALN, "%d, %d", numObjects, index);
		} break;

		case TUT_OP_GETLOCAL1:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_Push(vm, &vm->stack[vm->fp + index]);

			DEBUG_CYCLE(TUT_OP_GETLOCAL1, "%d", index);
		} break;

		case TUT_OP_SETLOCALN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;
			
			memcpy(&vm->stack[vm->fp + index], &vm->stack[vm->sp - numObjects], sizeof(TutObject) * numObjects);
			vm->sp -= numObjects;

			DEBUG_CYCLE(TUT_OP_SETLOCALN, "%d, %d", numObjects, index);
		} break;

		case TUT_OP_SETLOCAL1:
		{
			int32_t index = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc += 4;

			Tut_Pop(vm, &vm->stack[vm->fp + index]);
			
			DEBUG_CYCLE(TUT_OP_SETLOCAL1, "%d", index);
		} break;

		case TUT_OP_GETREFN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			uint16_t offset = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			TutObject* ref = Tut_PopRef(vm);

			memcpy(&vm->stack[vm->sp], &ref[offset], sizeof(TutObject) * numObjects);
			vm->sp += numObjects;

			DEBUG_CYCLE(TUT_OP_GETREFN, "%x, %d", (uintptr_t)ref, numObjects);
		} break;

		case TUT_OP_GETREF1:
		{
			uint16_t offset = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;
			
			TutObject* ref = Tut_PopRef(vm);
			
			Tut_Push(vm, &ref[offset]);

			DEBUG_CYCLE(TUT_OP_GETREF1, "%x", (uintptr_t)ref);
		} break;

		case TUT_OP_SETREFN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			uint16_t offset = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			TutObject* ref = Tut_PopRef(vm);

			memcpy(&ref[offset], &vm->stack[vm->sp - numObjects], sizeof(TutObject) * numObjects);
			vm->sp -= numObjects;

			DEBUG_CYCLE(TUT_OP_SETREFN, "%x, %d", (uintptr_t)ref, numObjects);
		} break;

		case TUT_OP_SETREF1:
		{
			uint16_t offset = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			TutObject* ref = Tut_PopRef(vm);
			Tut_Pop(vm, &ref[offset]);

			DEBUG_CYCLE(TUT_OP_SETREFN, "%x", (uintptr_t)ref);
		} break;

#define BIN_OP_INT(name, op) 
#define BIN_OP_FLOAT(name, op) case name: { float b = Tut_PopFloat(vm); float a = Tut_PopFloat(vm); Tut_PushFloat(vm, a op b); DEBUG_CYCLE(name, "%f, %f", a, b); } break;
		case TUT_OP_ADDI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a + b);

			DEBUG_CYCLE(TUT_OP_ADDI, "%d, %d", a, b);
		} break;
		
		case TUT_OP_SUBI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a - b); 

			DEBUG_CYCLE(TUT_OP_SUBI, "%d, %d", a, b);
		} break;

		case TUT_OP_MULI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a * b); 

			DEBUG_CYCLE(TUT_OP_MULI, "%d, %d", a, b);
		} break;
		
		case TUT_OP_DIVI: 
		{ 
			int32_t b = Tut_PopInt(vm); 
			int32_t a = Tut_PopInt(vm); 
			Tut_PushInt(vm, a / b); 

			DEBUG_CYCLE(TUT_OP_DIVI, "%d, %d", a, b);
		} break;
		
		BIN_OP_FLOAT(TUT_OP_ADDF, +)
		BIN_OP_FLOAT(TUT_OP_SUBF, -)
		BIN_OP_FLOAT(TUT_OP_MULF, *)
		BIN_OP_FLOAT(TUT_OP_DIVF, /)	

#undef BIN_OP_INT
#undef BIN_OP_FLOAT

		case TUT_OP_LAND:
		{
			TutBool b = Tut_PopBool(vm), a = Tut_PopBool(vm);
			Tut_PushBool(vm, a && b);

			DEBUG_CYCLE(TUT_OP_LAND, "%d, %d", a, b);
		} break;

		case TUT_OP_LOR:
		{
			TutBool b = Tut_PopBool(vm), a = Tut_PopBool(vm);
			Tut_PushBool(vm, a || b);

			DEBUG_CYCLE(TUT_OP_LOR, "%d, %d", a, b);
		} break;
		
		case TUT_OP_LNOT:
		{
			TutBool a = Tut_PopBool(vm);
			Tut_PushBool(vm, !a);

			DEBUG_CYCLE(TUT_OP_LNOT, "%d", a);
		} break;

		case TUT_OP_ILT:
		{
			int32_t b = Tut_PopInt(vm), a = Tut_PopInt(vm);
			Tut_PushBool(vm, a < b);

			DEBUG_CYCLE(TUT_OP_ILT, "%d, %d", a, b);
		} break;

		case TUT_OP_IGT:
		{
			int32_t b = Tut_PopInt(vm), a = Tut_PopInt(vm);
			Tut_PushBool(vm, a > b);

			DEBUG_CYCLE(TUT_OP_IGT, "%d, %d", a, b);
		} break;

		case TUT_OP_ILTE:
		{
			int32_t b = Tut_PopInt(vm), a = Tut_PopInt(vm);
			Tut_PushBool(vm, a <= b);

			DEBUG_CYCLE(TUT_OP_ILTE, "%d, %d", a, b);
		} break;

		case TUT_OP_IGTE:
		{
			int32_t b = Tut_PopInt(vm), a = Tut_PopInt(vm);
			Tut_PushBool(vm, a >= b);

			DEBUG_CYCLE(TUT_OP_IGTE, "%d, %d", a, b);
		} break;

		case TUT_OP_IEQ:
		{
			int32_t b = Tut_PopInt(vm), a = Tut_PopInt(vm);
			Tut_PushBool(vm, a == b);

			DEBUG_CYCLE(TUT_OP_IEQ, "%d, %d", a, b);
		} break;

		case TUT_OP_INEG:
		{
			int32_t i = Tut_PopInt(vm);
			Tut_PushInt(vm, -i);

			DEBUG_CYCLE(TUT_OP_INEG, "%d", i);
		} break;

		case TUT_OP_FLT: 
		{
			float b = Tut_PopFloat(vm), a = Tut_PopFloat(vm);
			Tut_PushBool(vm, a < b);

			DEBUG_CYCLE(TUT_OP_FLT, "%g, %g", a, b);
		} break;
		
		case TUT_OP_FGT:
		{
			float b = Tut_PopFloat(vm), a = Tut_PopFloat(vm);
			Tut_PushBool(vm, a > b);

			DEBUG_CYCLE(TUT_OP_FGT, "%g, %g", a, b);
		} break;
		
		case TUT_OP_FLTE:
		{
			float b = Tut_PopFloat(vm), a = Tut_PopFloat(vm);
			Tut_PushBool(vm, a <= b);

			DEBUG_CYCLE(TUT_OP_FLTE, "%g, %g", a, b);
		} break;

		case TUT_OP_FGTE:
		{
			float b = Tut_PopFloat(vm), a = Tut_PopFloat(vm);
			Tut_PushBool(vm, a >= b);

			DEBUG_CYCLE(TUT_OP_FGTE, "%g, %g", a, b);
		} break;
			
		case TUT_OP_FEQ:
		{
			float b = Tut_PopFloat(vm), a = Tut_PopFloat(vm);
			Tut_PushBool(vm, a == b);

			DEBUG_CYCLE(TUT_OP_FEQ, "%g, %g", a, b);
		} break;

		case TUT_OP_FNEG:
		{
			float f = Tut_PopFloat(vm);
			Tut_PushFloat(vm, -f);

			DEBUG_CYCLE(TUT_OP_FNEG, "%g", f);
		} break;

		case TUT_OP_BEQ:
		{
			TutBool b = Tut_PopBool(vm);
			TutBool a = Tut_PopBool(vm);

			Tut_PushBool(vm, a == b);

			DEBUG_CYCLE(TUT_OP_BEQ, "%s, %s", a ? "true" : "false", b ? "true" : "false");
		} break;

		case TUT_OP_SEQ:
		{
			const char* b = Tut_PopString(vm);
			const char* a = Tut_PopString(vm);

			Tut_PushBool(vm, strcmp(a, b) == 0);

			DEBUG_CYCLE(TUT_OP_SEQ, "%s, %s", a, b);
		} break;

		case TUT_OP_REQ:
		{
			const void* b = Tut_PopRef(vm);
			const void* a = Tut_PopRef(vm);

			Tut_PushBool(vm, a == b);
			
			DEBUG_CYCLE(TUT_OP_REQ, "%x, %x", (uintptr_t)a, (uintptr_t)b);
		} break;

		case TUT_OP_CALL:
		{
			uint16_t nargs = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			TutFunctionObject func = Tut_PopFunc(vm);

			if (!func.isExtern)
			{
				assert(func.index >= 0 && func.index < vm->functionPcs.length);

				TutReturnFrame frame;

				frame.nargs = nargs;
				frame.pc = vm->pc;
				frame.fp = vm->fp;

				Tut_ArrayPush(&vm->returnFrames, &frame);

				vm->pc = TUT_ARRAY_GET_VALUE(&vm->functionPcs, func.index, int32_t);
				vm->fp = vm->sp;

				DEBUG_CYCLE(TUT_OP_CALL, "%d, %d", func.index, nargs);
			}
			else
			{
				DEBUG_CYCLE(TUT_OP_CALL, "extern %s, %d", TUT_ARRAY_GET_VALUE(&vm->externNames, func.index, const char*), nargs);
				assert(func.index >= 0 && func.index < vm->externs.length);

				TutVMExternFunction ext = TUT_ARRAY_GET_VALUE(&vm->externs, func.index, TutVMExternFunction);

				int32_t sp = vm->sp;

				uint16_t numObjects = ext(vm, &vm->stack[sp - nargs], nargs);

				vm->sp = sp - nargs;

				memcpy(&vm->stack[vm->sp], &vm->stack[sp], sizeof(TutObject) * numObjects);
				vm->sp += numObjects;
			}
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

			vm->sp = vm->fp;
			vm->sp -= frame.nargs;
			vm->fp = frame.fp;
			vm->pc = frame.pc;

			DEBUG_CYCLE(TUT_OP_RET, "");
		} break;

		case TUT_OP_RETVALN:
		{
			uint16_t numObjects = Tut_ReadUint16(vm->code, vm->pc);
			vm->pc += 2;

			int copySp = vm->sp - numObjects;

			if (copySp < 0)
			{
				fprintf(stderr, "VM Stack Underflow (TUT_OP_RETVALN).\n");
				vm->pc = -1;
				return;
			}

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

			memcpy(&vm->stack[vm->sp], &vm->stack[copySp], sizeof(TutObject) * numObjects);
			vm->sp += numObjects;

			DEBUG_CYCLE(TUT_OP_RETVALN, "%d", numObjects);
		} break;

		case TUT_OP_RETVAL1:
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

			DEBUG_CYCLE(TUT_OP_RETVAL1, "");
		} break;

		case TUT_OP_GOTO:
		{
			int32_t pc = Tut_ReadInt32(vm->code, vm->pc);
			DEBUG_CYCLE(TUT_OP_GOTO, "%d", pc);
			vm->pc = pc;
		} break;

		case TUT_OP_GOTOFALSE:
		{
			int32_t pc = Tut_ReadInt32(vm->code, vm->pc);
			DEBUG_CYCLE(TUT_OP_GOTOFALSE, "%d", pc);

			vm->pc += 4;
			
			TutBool value = Tut_PopBool(vm);
			if(!value)
				vm->pc = pc;
		} break;

		case TUT_OP_HALT:
		{
			vm->pc = -1;
			DEBUG_CYCLE(TUT_OP_HALT, "");
		} break;

		default:
			DEBUG_CYCLE(INVALID_OP);
			break;
	}

}

void Tut_DestroyVM(TutVM* vm)
{
	// TODO: Implement this
}
