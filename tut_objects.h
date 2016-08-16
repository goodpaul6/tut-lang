#ifndef TUT_OBJECTS_H
#define TUT_OBJECTS_H

#include <stdint.h>

#include "tut_util.h"

typedef struct
{
	TutBool isExtern;
	int32_t index;
} TutFunctionObject;

typedef union
{
	TutBool bv;
	int32_t iv;
	float fv;
	const char* sv;
	void* ref;
	TutFunctionObject func;
} TutObject;

#endif
