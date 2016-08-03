#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tut_util.h"

void Tut_ErrorExit(const char* fmt, ...)
{
	fprintf(stderr, "Error: ");

	va_list args;
	
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	
	exit(TUT_FAILURE);
}

void* Tut_Malloc(size_t size)
{
	void* mem = malloc(size);
	if(!mem)
		Tut_ErrorExit("Out of memory!\n");
	return mem;
}

void* Tut_Calloc(size_t num, size_t size)
{
	void* mem = calloc(num, size);
	if(!mem)
		Tut_ErrorExit("Out of memory!\n");
	return mem;
}

void* Tut_Realloc(void* mem, size_t newSize)
{
	void* newMem = realloc(mem, newSize);
	if(!newMem)
		Tut_ErrorExit("Out of memory!\n");
	return newMem;
}

void Tut_Free(void* ptr)
{
	free(ptr);
}

char* Tut_Strdup(const char* string)
{
	size_t length = strlen(string);
	char* str = Tut_Malloc(length + 1);
	strcpy(str, string);
	
	return str;
}

int Tut_Strncmp(const char* a, const char* b, uint32_t length)
{
	uint32_t i = 0;
	while((*a++ == *b++) && i < length)
		++i;
	
	return *a - *b;
}
