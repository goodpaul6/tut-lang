#include <string.h>

#include "tut_typetag.h"

static const char* Names[TUT_TYPETAG_COUNT] = 
{
	"bool",
	"int",
	"float",
	"cstr",
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
		Tut_InitList(&tag->user.members);
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
		
		TUT_LIST_EACH(node, tag->user.members)
			Tut_DestroyTypetag(node->value);
		Tut_DestroyList(&tag->user.members);
	}
}
