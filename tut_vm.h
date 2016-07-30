#ifndef TUT_VM_H
#define TUT_VM_H

#define TUT_VM_MAX_CODE_SIZE	4096
#define TUT_VM_MAX_DATA_SIZE	4096
#define TUT_VM_MAX_STACK_SIZE	2048

#include <stdint.h>

typedef struct
{
	int32_t sp, pc, fp;
	
	uint8_t code[TUT_VM_MAX_CODE_SIZE];
	uint8_t data[TUT_VM_MAX_DATA_SIZE];
	uint8_t stack[TUT_VM_MAX_STACK_SIZE];
} TutVM;

void Tut_InitVM(TutVM* vm);

void Tut_Push(TutVM* vm, const void* value, uint32_t size);
void Tut_Pop(TutVM* vm, void* value, uint32_t size);

void Tut_PushBool(TutVM* vm, TutBool value);
void Tut_PushInt(TutVM* vm, int32_t value);
void Tut_PushFloat(TutVM* vm, float value);

// NOTE: Does not make a copy of the string
void Tut_PushCString(TutVM* vm, uint32_t length, const char* string);

// NOTE: Makes a copy of string
void Tut_PushString(TutVM* vm, uint32_t length, const char* string);

TutBool Tut_PopBool(TutVM* vm);
int32_t Tut_PopInt(TutVM* vm);
float Tut_PopFloat(TutVM* vm);
TutCStringObject Tut_PopCString(TutVM* vm);
TutStringObject TutPopString(TutVM* vm);

void Tut_ExecuteCycle(TutVM* vm);
void Tut_DestroyVM(TutVM* vm);

#endif
