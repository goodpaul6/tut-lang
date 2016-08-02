#include "tut_util.h"
#include "tut_list.h"

static TutListNode* MakeNode(void* value)
{
	TutListNode* node = Tut_Malloc(sizeof(TutListNode));
	
	node->value = value;
	node->next = NULL;
	node->prev = NULL;

	return node;
}

void Tut_InitList(TutList* list)
{
	list->length = 0;
	list->head = list->tail = NULL;
}

void Tut_ListAppend(TutList* list, void* value)
{
	TutListNode* node = MakeNode(value);
	
	if(!list->head)
		list->head = list->tail = node;
	else
	{
		node->prev = list->tail;
		list->tail->next = node;
		list->tail = node;
	}
	
	++list->length;
}

void Tut_ListPrepend(TutList* list, void* value)
{
	TutListNode* node = MakeNode(value);
	
	if(!list->head)
		list->head = list->tail = node;
	else
	{
		list->head->prev = node;
		node->next = list->head;
		list->head = node;
	}
	
	++list->length;
}

void Tut_DestroyList(TutList* list)
{
	TUT_LIST_EACH(node, *list)
		Tut_Free(node);
	
	list->length = 0;
	list->head = list->tail = NULL;
}
