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
	"cstr",
	"ref",
	"ptr",
	"func",
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
	else if (tag->type == TUT_TYPETAG_FUNC)
	{
		Tut_InitList(&tag->func.args);
		tag->func.ret = NULL;
		tag->func.hasVarargs = TUT_FALSE;
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
		// IMPORTANT:
		// ref-x == ref-x
		
		// THESE ARE HANDLED IN THE COMPILER:
		// here:
		// ref != ref-x
		// ref-x != ref
		// but the compiler allows (when passing arguments or assigning):
		// ref == ref-x
		// ref-x == ref where x is not a usertype

		// ref == ref
		if (a->ref.value && b->ref.value)
			return Tut_CompareTypes(a->ref.value, b->ref.value);
		else if (a->ref.value || b->ref.value)
			return TUT_FALSE;
		return TUT_TRUE;
	}
	else if (a->type == TUT_TYPETAG_FUNC)
	{
		if (Tut_CompareTypes(a->func.ret, b->func.ret))
		{
			TutListNode* aNode = a->func.args.head;
			TutListNode* bNode = b->func.args.head;

			while (aNode && bNode)
			{
				TutTypetag* aTag = aNode->value;
				TutTypetag* bTag = bNode->value;

				if (!Tut_CanAssignTypes(aTag, bTag))
					return TUT_FALSE;

				aNode = aNode->next;
				bNode = bNode->next;
			}
				
			// func(x, ...) == func(x, y, z)
			// func(x, y, z) == func(x, ...)
			if ((bNode && !aNode) || (aNode && !bNode))
			{
				if (a->func.hasVarargs || b->func.hasVarargs)
					return TUT_TRUE;

				return TUT_FALSE;
			}
			
			return TUT_TRUE;
		}
	}
	
	return TUT_TRUE;	
}

TutBool Tut_CanAssignTypes(const TutTypetag* from, const TutTypetag* to)
{
	if (!Tut_CompareTypes(to, from))
	{
		// str -> cstr
		if (to->type == TUT_TYPETAG_CSTR && from->type == TUT_TYPETAG_STR)
			return TUT_TRUE;
		// ref -> ref-x
		else if ((to->type == TUT_TYPETAG_REF && to->ref.value) &&
			(from->type == TUT_TYPETAG_REF && !from->ref.value))
			return TUT_TRUE;
		// ref-x -> ref where x is not usertype
		else if ((to->type == TUT_TYPETAG_REF && !to->ref.value) &&
			(from->type == TUT_TYPETAG_REF && from->ref.value->type != TUT_TYPETAG_USERTYPE))
			return TUT_TRUE;

		return TUT_FALSE;
	}

	return TUT_TRUE;
}

const char* Tut_TypetagRepr(const TutTypetag* tag)
{
	switch (tag->type)
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

		case TUT_TYPETAG_FUNC:
		{
			// TODO: Fix potential buffer overflow lol nope
			static char buf[1024];

			int len = sprintf(buf, "func(");

			TUT_LIST_EACH(node, tag->func.args) 
			{
				len += sprintf(buf + len, "%s", Tut_TypetagRepr(node->value));
				if (node->next)
					len += sprintf(buf + len, ", ");
			}

			sprintf(buf + len, ")-%s", Tut_TypetagRepr(tag->func.ret));

			return Tut_Strdup(buf);
		} break;

		default:
			return Names[(int)tag->type];
	}
}

void Tut_DeleteTypetag(TutTypetag* tag)
{
	if (tag->type == TUT_TYPETAG_USERTYPE)
	{
		if (tag->user.name) Tut_Free(tag->user.name);

		for (size_t i = 0; i < tag->user.members.length; ++i)
		{
			TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);

			Tut_Free(mem->name);
			Tut_DeleteTypetag(mem->typetag);
		}
		Tut_DestroyArray(&tag->user.members);
	}
	else if (tag->type == TUT_TYPETAG_REF && tag->ref.value)
		Tut_DeleteTypetag(tag->ref.value);
	Tut_Free(tag);
}
