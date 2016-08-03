#include <string.h>

#include "tut_typetag.h"

static const char* Names[TUT_TYPETAG_COUNT] = 
{
	"void",
	"bool",
	"int",
	"float",
	"str",
	NULL
};

void Tut_InitTypetag(TutTypetag* tag, TutTypetagType type)
{
	tag->type = type;
	
	if(tag->type == TUT_TYPETAG_USERTYPE)
	{
		tag->user.defined = TUT_FALSE;
		tag->user.name = NULL;
		Tut_InitArray(&tag->user.members, sizeof(TutTypetagMember));
	}
}

TutTypetag* Tut_CreatePrimitiveTypetag(const char* name)
{
	for(int i = 0; i < TUT_TYPETAG_USERTYPE; ++i)
	{
		if(strcmp(name, Names[i]) == 0)
		{
			TutTypetag* tag = Tut_Malloc(sizeof(TutTypetag));
			Tut_InitTypetag(tag, (TutTypetagType)i);
			return tag;
		}
	}
	
	return NULL;
}

int Tut_GetTypetagCount(TutTypetag* tag)
{
	if (tag->type == TUT_TYPETAG_USERTYPE)
	{
		int totalSize = 0;
		for (size_t i = 0; i < tag->user.members.length; ++i)
		{
			TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);
			totalSize += Tut_GetTypetagCount(mem->typetag);
		}

		return totalSize;
	}
	else if (tag->type == TUT_TYPETAG_VOID)
		return 0;

	return 1;
}

TutBool Tut_CompareTypes(const TutTypetag* a, const TutTypetag* b)
{
	if(a->type != b->type) return TUT_FALSE;
	
	if(a->type == TUT_TYPETAG_USERTYPE)
	{
		if(a->user.name && b->user.name)
			return strcmp(a->user.name, b->user.name) == 0;
		return TUT_FALSE;
	}
	
	return TUT_TRUE;	
}

const char* Tut_TypetagRepr(const TutTypetag* tag)
{
	switch(tag->type)
	{
		case TUT_TYPETAG_USERTYPE: return tag->user.name;
		
		default:
			return Names[(int)tag->type];
	}
}

void Tut_DestroyTypetag(TutTypetag* tag)
{
	if(tag->type == TUT_TYPETAG_USERTYPE)
	{
		if(tag->user.name) Tut_Free(tag->user.name);
		
		for (size_t i = 0; i < tag->user.members.length; ++i)
		{
			TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);

			Tut_Free(mem->name);
			Tut_DestroyTypetag(mem->typetag);
		}
		Tut_DestroyArray(&tag->user.members);
	}
}
