#ifndef TUT_LIST_H
#define TUT_LIST_H

#include <stddef.h>

#define TUT_LIST_EACH(nodeName, list) for(TutListNode* nodeName = (list).head; nodeName; nodeName = nodeName->next)
#define TUT_LIST_REVERSE_EACH(nodeName, list) for(TutListNode* node = (list).tail; nodeName; nodeName = nodeName->prev)

typedef struct TutListNode
{
	struct TutListNode* next;
	struct TutListNode* prev;
	void* value;
} TutListNode;

typedef struct
{
	size_t length;
	TutListNode* head;
	TutListNode* tail;
} TutList;

void Tut_InitList(TutList* list);

void Tut_ListAppend(TutList* list, void* value);
void Tut_ListPrepend(TutList* list, void* value);

void Tut_DestroyList(TutList* list);

#endif
