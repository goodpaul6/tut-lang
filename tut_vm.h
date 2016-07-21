#ifndef TUT_VM_H
#define TUT_VM_H

#define TUT_VM_MAX_DATA_SIZE	4096
#define TUT_VM_MAX_STACK_SIZE	2048

#include "tut_array.h"
#include "tut_objects.h"

typedef struct
{
	int32_t sp, pc, fp;
	
	TutArray code;
	
	char data[TUT_VM_MAX_DATA_SIZE];
	char stack[TUT_VM_MAX_STACK_SIZE];
} TutVM;

void Tut_InitVM(TutVM* vm);

void Tut_Push(TutVM* vm, const void* value, uint32_t size);
void Tut_PushBool(TutVM* vm, TutBool value);
void Tut_PushInt(TutVM* vm, int32_t value);
void Tut_PushFloat(TutVM* vm, float value);
void Tut_PushString(TutVM* vm, uint32_t length, const char* string);

void Tut_ExecuteCycle(TutVM* vm);
void Tut_DestroyVM(TutVM* vm);

#endif
