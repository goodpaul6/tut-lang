#ifndef TUT_CODEGEN_H
#define TUT_CODEGEN_H

#include "tut_array.h"
#include "tut_vm.h"
#include "tut_expr.h"
#include "tut_module.h"

#define TUT_CODEGEN_MAX_GLOBALS		64
#define TUT_CODEGEN_MAX_FUNCTIONS	128
#define TUT_CODEGEN_MAX_INTEGERS	256
#define TUT_CODEGEN_MAX_FLOATS		256
#define TUT_CODEGEN_MAX_STRINGS		128

typedef struct
{
	uint32_t numFunctions;
	uint32_t functionOffsets[TUT_CODEGEN_MAX_FUNCTIONS];
	
	uint32_t numGlobals;
	uint32_t globalOffsets[TUT_CODEGEN_MAX_GLOBALS];
	
	uint32_t numIntegers;
	uint32_t integerOffsets[TUT_CODEGEN_MAX_INTEGERS];
	
	uint32_t numFloats;
	uint32_t floatOffsets[TUT_CODEGEN_MAX_FLOATS];
	
	uint32_t numStrings;
	uint32_t stringOffsets[TUT_CODEGEN_MAX_STRINGS];
	
	uint32_t dataLength;
	uint8_t data[TUT_VM_MAX_DATA_SIZE];
	
	size_t codeLength;
	uint8_t code[TUT_VM_MAX_CODE_SIZE];
} TutCodegen;

void Tut_InitCodegen(TutCodegen* gen);
void Tut_CompileValueExpr(TutModule* module, TutCodegen* gen, TutExpr* exp);
void Tut_DestroyCodegen(TutCodegen* gen);

#endif
