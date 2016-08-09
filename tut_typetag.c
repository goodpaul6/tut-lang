#include <string.h>
#include <stdio.h>

#include "tut_typetag.h"

static const char* Names[TUT_TYPETAG_COUNT] = 
{
	"void",
	"bool",
	"int",
	"float",
	"str",
	"ref",
	NULL
};

void Tut_InitTypetag(TutTypetag* tag, TutTypetagType type)
{
	tag->type = type;
	
	if (tag->type == TUT_TYPETAG_USERTYPE)
	{
		tag->user.defined = TUT_FALSE;
		tag->user.name = NULL;
		Tut_InitArray(&tag->user.members, sizeof(TutTypetagMember));
	}
	else if (tag->type == TUT_TYPETAG_REF)
		tag->ref.value = NULL;
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

int Tut_GetTypetagSize(TutTypetag* tag)
{
	if (tag->type == TUT_TYPETAG_USERTYPE)
	{
		int totalSize = 0;
		for (size_t i = 0; i < tag->user.members.length; ++i)
		{
			TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);
			totalSize += Tut_GetTypetagSize(mem->typetag);
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
	else if (a->type == TUT_TYPETAG_REF)
	{
		if (a->ref.value && b->ref.value)
			return Tut_CompareTypes(a->ref.value, b->ref.value);
		return TUT_FALSE;
	}
	
	return TUT_TRUE;	
}

const char* Tut_TypetagRepr(const TutTypetag* tag)
{
	switch(tag->type)
	{
		case TUT_TYPETAG_USERTYPE: return tag->user.name;
		case TUT_TYPETAG_REF:
		{
			static char buf[1024];
			
			if (tag->ref.value)
				sprintf(buf, "ref-%s", Tut_TypetagRepr(tag->ref.value));
			else
				sprintf(buf, "ref");
			
			// TODO: Fix memory leak (LOL)
			return Tut_Strdup(buf);
		} break;

		default:
			return Names[(int)tag->type];
	}
}

void Tut_DestroyTypetag(TutTypetag* tag)
{
	if (tag->type == TUT_TYPETAG_USERTYPE)
	{
		if (tag->user.name) Tut_Free(tag->user.name);

		for (size_t i = 0; i < tag->user.members.length; ++i)
		{
			TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);

			Tut_Free(mem->name);
			Tut_DestroyTypetag(mem->typetag);
		}
		Tut_DestroyArray(&tag->user.members);
	}
	else if (tag->type == TUT_TYPETAG_REF && tag->ref.value)
		Tut_DestroyTypetag(tag->ref.value);
}
