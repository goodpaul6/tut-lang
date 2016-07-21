#include <stdio.h>

#include "tut_vm.h"

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

void Tut_PushBool(TutVM* vm, TutBool value);
void Tut_PushInt(TutVM* vm, int32_t value);
void Tut_PushFloat(TutVM* vm, float value);
void Tut_PushString(TutVM* vm, uint32_t length, const char* string);

void Tut_ExecuteCycle(TutVM* vm)
{
	if(vm->pc < 0) return;
	
	uint8_t op = vm->array.data[vm->pc];
	
	switch(op)
	{
		case TUT_OP_PUSH_BOOL:
		{
			
		} break;
	}
}

void Tut_DestroyVM(TutVM* vm)
{
	
}
