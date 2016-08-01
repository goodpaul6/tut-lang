#ifndef TUT_ARRAY_H
#define TUT_ARRAY_H

#include <stddef.h>

#define TUT_ARRAY_INIT_CAPACITY	8

typedef struct 
{
	unsigned char* data;
	size_t datumSize;
	
	size_t length;
	size_t capacity;
} TutArray;

typedef int (*Tut_ArrayComparator)(const void* a, const void* b);
typedef void (*Tut_ArrayTraverser)(void* data, void* value);

void Tut_InitArray(TutArray* array, size_t datumSize);

void Tut_ArrayCopy(TutArray* from, TutArray* to);

void Tut_ArrayReserve(TutArray* array, size_t capacity);
void Tut_ArrayResize(TutArray* array, size_t length, const void* init);

void* Tut_ArrayGet(TutArray* array, size_t index);
#define TUT_ARRAY_GET_VALUE(array, index, type) (*(type*)Tut_ArrayGet((array), (index)))

void Tut_ArraySet(TutArray* array, size_t index, const void* value);

void Tut_ArrayPush(TutArray* array, const void* value);
void Tut_ArrayPop(TutArray* array, void* value);

void Tut_ArrayInsert(TutArray* array, size_t index, const void* value);

void Tut_ArrayTraverse(TutArray* array, Tut_ArrayTraverser traverser, void* data);

void* Tut_GetArraySortData(TutArray* array);
void Tut_ArraySort(TutArray* array, Tut_ArrayComparator comparator, void* data);

void Tut_ArrayClear(TutArray* array);

void Tut_DestroyArray(TutArray* array);

#endif
