#ifndef TUT_OBJECTS_H
#define TUT_OBJECTS_H

#include <stdint.h>

typedef struct
{
	int8_t value;
} TutBoolObject;

typedef struct
{
	int32_t value;
} TutIntObject;

typedef struct
{
	float value;
} TutFloatObject;

typedef struct
{
	uint32_t length;
	char* data;
} TutStringObject;

typedef struct
{
	uint32_t length;
	const char* data;
} TutCStringObject;

#endif
