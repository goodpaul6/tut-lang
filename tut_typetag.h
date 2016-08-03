#ifndef TUT_TYPETAG_H
#define TUT_TYPETAG_H

#include "tut_list.h"
#include "tut_util.h"
#include "tut_array.h"

typedef enum
{
	TUT_TYPETAG_VOID,
	TUT_TYPETAG_BOOL,
	TUT_TYPETAG_INT,
	TUT_TYPETAG_FLOAT,
	TUT_TYPETAG_CSTR,
	TUT_TYPETAG_STR,
	TUT_TYPETAG_USERTYPE,
	TUT_TYPETAG_COUNT
} TutTypetagType;

struct TutTypetag;

typedef struct
{
	char* name;
	struct TutTypetag* typetag;
} TutTypetagMember;

typedef struct TutTypetag
{
	TutTypetagType type;
	
	union
	{
		struct
		{
			TutBool defined;
			char* name;
			TutArray members;
		} user;
	};
} TutTypetag;

void Tut_InitTypetag(TutTypetag* tag, TutTypetagType type);
TutTypetag* Tut_CreatePrimitiveTypetag(const char* name);

int Tut_GetTypetagCount(TutTypetag* tag);

TutBool Tut_CompareTypes(const TutTypetag* a, const TutTypetag* b); 
const char* Tut_TypetagRepr(const TutTypetag* tag);

void Tut_DestroyTypetag(TutTypetag* tag);

#endif
