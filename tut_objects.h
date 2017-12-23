#ifndef TUT_OBJECTS_H
#define TUT_OBJECTS_H

#include <stdint.h>

#include "tut_util.h"

typedef enum
{
	TUT_OBJECT_BOOL,
	TUT_OBJECT_INT,
	TUT_OBJECT_FLOAT,
	TUT_OBJECT_STR,
	TUT_OBJECT_CSTR,
	TUT_OBJECT_REF,
	TUT_OBJECT_PTR,
	TUT_OBJECT_FUNC
} TutObjectType;

typedef struct
{
	TutBool isExtern;
	int32_t index;
} TutFunctionObject;

typedef struct TutObject
{
	uint8_t type;
	union
	{
		int32_t bv;
		int32_t iv;
		float fv;
		char* sv;
		struct TutObject* ref;
		void* ptr;
		TutFunctionObject func;
	};
} TutObject;

#endif
