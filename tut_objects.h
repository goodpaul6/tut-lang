#ifndef TUT_OBJECTS_H
#define TUT_OBJECTS_H

#include <stdint.h>

#include "tut_util.h"

typedef union
{
	TutBool bv;
	int32_t iv;
	float fv;
	const char* sv;
	void* ref;
} TutObject;

#endif
