#include <stdio.h>
#include <assert.h>

#include "tut_vm.h"
#include "tut_objects.h"
#include "tut_buf.h"

void Tut_InitVM(TutVM* vm)
{
	vm->sp = 0;
	vm->pc = -1;
	vm->fp = 0;

	Tut_InitArray(&vm->code, 1);
}

void Tut_Push(TutVM* vm, const void* value, uint32_t size)
{
	if(vm->sp + size >= TUT_VM_MAX_STACK_SIZE)
	{
		fprintf(stderr, "VM Stack Overflow!\n");
		vm->pc = -1;
	}
	
	memcpy(&vm->stack[vm->sp], value, size);
	vm->sp += size;
}

void Tut_Pop(TutVM* vm, void* value, uint32_t size)
{
	if(vm->sp - size < 0)
	{
		fprintf(stderr, "VM Stack Underflow!\n");
		vm->pc = -1;
	}
	
	memcpy(value, &vm->stack[vm->sp], size);
	vm->sp -= size;
}

void Tut_PushBool(TutVM* vm, TutBool value)
{
	TutBoolObject object = { value };
	Tut_Push(vm, &object, sizeof(object));
}

void Tut_PushInt(TutVM* vm, int32_t value)
{
	TutIntObject object = { value };
	Tut_Push(vm, &object, sizeof(object));
}

void Tut_PushFloat(TutVM* vm, float value)
{
	TutFloatObject object = { value };
	Tut_Push(vm, &object, sizeof(object));
}

void Tut_PushCString(TutVM* vm, uint32_t length, const char* string)
{
	TutCStringObject object = { length, string };
	Tut_Push(vm, &object, sizeof(object));
}

void Tut_PushString(TutVM* vm, uint32_t length, const char* string)
{
	char* str = Tut_Strdup(string);
	TutStringObject object = { length, str };
	Tut_Push(vm, &object, sizeof(object));
}

TutBool Tut_PopBool(TutVM* vm)
{
	TutBoolObject object;
	
	Tut_Pop(vm, &object, sizeof(object));
	return (TutBool)object.value;
}

int32_t Tut_PopInt(TutVM* vm)
{
	TutIntObject object;
	
	Tut_Pop(vm, &object, sizeof(object));
	return (TutBool)object.value;
}

float Tut_PopFloat(TutVM* vm)
{
	TutFloatObject object;
	
	Tut_Pop(vm, &object, sizeof(object));
	return object.value;
}

TutCStringObject Tut_PopCString(TutVM* vm)
{
	TutCStringObject object;
	
	Tut_Pop(vm, &object, sizeof(object));
	return object;
}

TutStringObject TutPopString(TutVM* vm)
{
	TutStringObject object;
	
	Tut_Pop(vm, &object, sizeof(object));
	return object;
}

void Tut_ExecuteCycle(TutVM* vm)
{
	if(vm->pc < 0) return;
	
	uint8_t op = vm->code[vm->pc++];
	
	switch(op)
	{
		case TUT_OP_PUSH_BOOL:
		{
			TutBool value = (TutBool)vm->code[vm->pc++];
			Tut_PushBool(vm, value);
		} break;
		
		case TUT_OP_PUSH_INT:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			int32_t value = Tut_ReadInt32(vm->data, dataIndex);
			Tut_PushInt(vm, value);
		} break;
		
		case TUT_OP_PUSH_FLOAT:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			float value = Tut_ReadFloat(vm->data, dataIndex);
			Tut_PushFloat(vm, value);
		} break;
		
		case TUT_OP_PUSH_CSTR:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			uint32_t length = Tut_ReadUint32(vm->data, dataIndex);
			const char* data = &vm->data[dataIndex + 4];
			
			Tut_PushCString(vm, length, data);
		} break;
		
#define OP_POP(name, type) case name: { vm->sp -= sizeof(type); } break;
		
		OP_POP(TUT_OP_POP_BOOL, TutBoolObject)
		OP_POP(TUT_OP_POP_INT, TutIntObject)
		OP_POP(TUT_OP_POP_FLOAT, TutFloatObject)
		OP_POP(TUT_OP_POP_CSTR, TutCStringObject)
		OP_POP(TUT_OP_POP_STR, TutStringObject)
	
#undef OP_POP
		
		case TUT_OP_GET_GLOBAL_BOOL:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			TutBool value = (TutBool)vm->data[dataIndex];
			Tut_PushBool(vm, value);
		} break;
		
		case TUT_OP_GET_GLOBAL_INT:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			int32_t value = Tut_ReadInt32(vm->data, dataIndex);
			Tut_PushInt(vm, value);
		} break;
		
		case TUT_OP_GET_GLOBAL_FLOAT:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			float value = Tut_ReadFloat(vm->data, dataIndex);
			Tut_PushFloat(vm, value);
		} break;
		
		case TUT_OP_GET_GLOBAL_CSTR:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			uint32_t length = Tut_ReadUint32(vm->data, dataIndex);
			const char* str = &vm->data[dataIndex + 4];
			
			Tut_PushCString(vm, length, str);
		} break;
		
		case TUT_OP_GET_GLOBAL_STR:
		{
			uint32_t dataIndex = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			uint32_t length = Tut_ReadUint32(vm->data, dataIndex);
			const char* str = &vm->data[dataIndex + 4];
			
			Tut_PushString(vm, length, str);
		} break;
			
		case TUT_OP_GET_LOCAL_BOOL:
		{
			uint32_t offset = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			TutBool value = (TutBool)vm->stack[vm->fp + offset];
			Tut_PushBool(vm, value);
		} break;
		
		case TUT_OP_GET_LOCAL_INT:
		{
			uint32_t offset = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			int32_t value = Tut_ReadInt32(vm->stack, vm->fp + offset);
			Tut_PushInt(vm, value);
		} break;
		
		case TUT_OP_GET_LOCAL_FLOAT:
		{
			uint32_t offset = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			float value = Tut_ReadFloat(vm->stack, vm->fp + offset);
			Tut_PushFloat(vm, value);
		} break;
		
		case TUT_OP_GET_LOCAL_CSTR:
		{
			uint32_t offset = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			uint32_t length = Tut_ReadUint32(vm->stack, vm->pc + offset);
			const char* string = &vm->stack[vm->fp + offset + 4];
			
			Tut_PushCString(vm, length, string);
		} break;
		
		case TUT_OP_GET_LOCAL_STR:
		{
			uint32_t offset = Tut_ReadUint32(vm->code, vm->pc);
			vm->pc += 4;
			
			uint32_t length = Tut_ReadUint32(vm->stack, vm->pc + offset);
			const char* string = &vm->stack[vm->fp + offset + 4];
			
			Tut_PushString(vm, length, string);
		} break;
		
#define BIN_OP_INT(name, operator) case name: { int32_t b = Tut_PopInt(vm); int32_t a = Tut_PopInt(vm); Tut_PushInt(vm, a operator b); } break;
#define BIN_OP_FLOAT(name, operator) case name: { float b = Tut_PopFloat(vm); float a = Tut_PopFloat(vm); Tut_PushFloat(vm, a operator b); } break;
		
		BIN_OP_INT(TUT_OP_ADDI, +)
		BIN_OP_INT(TUT_OP_SUBI, -)
		BIN_OP_INT(TUT_OP_MULI, *)
		BIN_OP_INT(TUT_OP_DIVI, /)
		
		BIN_OP_FLOAT(TUT_OP_ADDF, +)
		BIN_OP_FLOAT(TUT_OP_ADDF, -)
		BIN_OP_FLOAT(TUT_OP_ADDF, *)
		BIN_OP_FLOAT(TUT_OP_ADDF, /)	

#undef BIN_OP_INT
#undef BIN_OP_FLOAT

		case TUT_OP_GOTO:
		{
			uint32_t pc = Tut_ReadInt32(vm->code, vm->pc);
			vm->pc = pc;
		} break;

		case TUT_OP_GOTOFALSE:
		{
			uint32_t pc = Tut_ReadInt32(vm->code, vm->pc);
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
