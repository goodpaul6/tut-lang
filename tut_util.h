#ifndef TUT_UTIL_H
#define TUT_UTIL_H

#include <stddef.h>

typedef enum
{
	TUT_FALSE = 0,
	TUT_TRUE = 1
} TutBool;

typedef enum
{
	TUT_SUCCESS = 0,
	TUT_FAILURE = 1,
} TutProgramStatus;

void Tut_ErrorExit(const char* fmt, ...);

void* Tut_Malloc(size_t size);
void* Tut_Calloc(size_t num, size_t size);
void* Tut_Realloc(void* mem, size_t newSize);

void Tut_Free(void* ptr);
char* Tut_Strdup(const char* string);
int Tut_Strncmp(const char* a, const char* b, uint32_t length);

#endif
