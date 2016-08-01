#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tut_array.h"
#include "tut_util.h"

static void* ArraySortData = NULL;

static void Expand(TutArray* array)
{
	array->data = Tut_Realloc(array->data, array->capacity * array->datumSize);
}

void Tut_InitArray(TutArray* array, size_t datumSize)
{
	array->data = NULL;
	array->datumSize = datumSize;
	array->length = 0;
	array->capacity = 0;
}

void Tut_ArrayCopy(TutArray* from, TutArray* to)
{
	assert(from->datumSize == to->datumSize);
	
	Tut_ArrayReserve(to, from->length);
	
	memcpy(to->data, from->data, to->datumSize * from->length);
	to->length = from->length;
}

void Tut_ArrayReserve(TutArray* array, size_t capacity)
{
	if(array->capacity < capacity)
	{
		array->capacity = capacity;
		Expand(array);
	}
}

void Tut_ArrayResize(TutArray* array, size_t length, const void* init)
{
	Tut_ArrayReserve(array, length);
	
	size_t prevLength = array->length;
	array->length = length;
	
	if(init)
	{
		for(int i = prevLength; i < length; ++i)
			memcpy(&array->data[i * array->datumSize], init, array->datumSize);
	}
}

void* Tut_ArrayGet(TutArray* array, size_t index)
{
	assert(index >= 0 && index < array->length);
	return &array->data[index * array->datumSize];
}

void Tut_ArraySet(TutArray* array, size_t index, const void* value)
{
	assert(index >= 0 && index < array->length);
	memcpy(&array->data[index * array->datumSize], value, array->datumSize);
}

void Tut_ArrayPush(TutArray* array, const void* value)
{
	while(array->length + 1 >= array->capacity)
	{
		if(array->capacity == 0)
			array->capacity = TUT_ARRAY_INIT_CAPACITY;
		else
			array->capacity *= 2;
		Expand(array);
	}
	
	memcpy(&array->data[array->length * array->datumSize], value, array->datumSize);
	array->length += 1;
}

void Tut_ArrayPop(TutArray* array, void* value)
{
	assert(array->length > 0);
	memcpy(value, &array->data[(--array->length) * array->datumSize], array->datumSize);
}

void Tut_ArrayInsert(TutArray* array, size_t index, const void* value)
{
	assert(index >= 0 && index < array->length);
	
	while(array->length + 1 >= array->capacity)
	{
		array->capacity *= 2;
		Expand(array);
	}
	
	memmove(&array->data[(index + 1) * array->datumSize], &array->data[index * array->datumSize], (array->length - index) * array->datumSize);
	memcpy(&array->data[index * array->datumSize], value, array->datumSize);
	array->length += 1;
}

void Tut_ArrayTraverse(TutArray* array, Tut_ArrayTraverser traverser, void* data)
{
	for(int i = 0; i < array->length; ++i)
		traverser(data, &array->data[i * array->datumSize]);
}

void* Tut_GetArraySortData(TutArray* array)
{
	return ArraySortData;
}

void Tut_ArraySort(TutArray* array, Tut_ArrayComparator comparator, void* data)
{
	ArraySortData = data;
	qsort(array->data, array->length, array->datumSize, (int(*)(const void*, const void*))comparator);
}

void Tut_ArrayClear(TutArray* array)
{
	array->length = 0;
}

void Tut_DestroyArray(TutArray* array)
{
	Tut_Free(array->data);
	
	array->length = 0;
	array->capacity = 0;
	array->datumSize = 0;
}
