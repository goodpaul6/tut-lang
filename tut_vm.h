#ifndef TUT_VM_H
#define TUT_VM_H

#define TUT_VM_MAX_CODE_SIZE	4096
#define TUT_VM_MAX_GLOBALS		256	
#define TUT_VM_STACK_SIZE		256

#include "tut_util.h"
#include "tut_objects.h"
#include "tut_array.h"

typedef struct
{
	uint8_t nargs;
	int32_t pc, fp;
} TutReturnFrame;

typedef struct
{
	int32_t sp, pc, fp;
	
	TutArray integers;
	TutArray floats;
	TutArray strings;
	
	TutArray functionPcs;
	TutArray externs;

	TutArray returnFrames;

	uint32_t codeSize;
	uint8_t code[TUT_VM_MAX_CODE_SIZE];

	TutObject globals[TUT_VM_MAX_GLOBALS];
	TutObject stack[TUT_VM_STACK_SIZE];
} TutVM;

// Externs return true if they return a value (i.e push onto the stack)
// or false if they don't
typedef TutBool(*TutVMExternFunction)(TutVM* vm, uint8_t nargs);

void Tut_InitVM(TutVM* vm);

void Tut_Push(TutVM* vm, const TutObject* value);
void Tut_Pop(TutVM* vm, TutObject* object);

void Tut_PushBool(TutVM* vm, TutBool value);
void Tut_PushInt(TutVM* vm, int32_t value);
void Tut_PushFloat(TutVM* vm, float value);

// NOTE: Does not make a copy of the string
void Tut_PushCString(TutVM* vm, const char* string);

// NOTE: Makes a copy of string
void Tut_PushString(TutVM* vm, const char* string);

TutBool Tut_PopBool(TutVM* vm);
int32_t Tut_PopInt(TutVM* vm);
float Tut_PopFloat(TutVM* vm);
const char* Tut_PopCString(TutVM* vm);
char* TutPopString(TutVM* vm);

void Tut_BindExtern(TutVM* vm, TutVMExternFunction ext, uint32_t index);

void Tut_ExecuteCycle(TutVM* vm, TutBool printOp);

void Tut_DestroyVM(TutVM* vm);

#endif
