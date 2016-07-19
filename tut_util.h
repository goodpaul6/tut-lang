#ifndef TUT_UTIL_H
#define TUT_UTIL_H

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
void Tut_Free(void* ptr);
char* Tut_Strdup(const char* string);

#endif
