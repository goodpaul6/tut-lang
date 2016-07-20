#ifndef TUT_TYPETAG_H
#define TUT_TYPETAG_H

#include "tut_list.h"
#include "tut_util.h"

typedef enum
{
	TUT_TYPETAG_BOOL,
	TUT_TYPETAG_INT,
	TUT_TYPETAG_FLOAT,
	TUT_TYPETAG_CSTR,
	TUT_TYPETAG_STR,
	TUT_TYPETAG_USERTYPE,
	TUT_TYPETAG_COUNT
} TutTypetagType;

typedef struct
{
	TutTypetagType type;
	
	struct
	{
		TutBool defined;
		char* name;
		TutList members;
	} user;
} TutTypetag;

void Tut_InitTypetag(TutTypetag* tag, TutTypetagType type);
TutTypetag* Tut_CreatePrimitiveTypetag(const char* name);

TutBool Tut_CompareTypes(const TutTypetag* a, const TutTypetag* b); 
const char* Tut_TypetagRepr(const TutTypetag* tag);

void Tut_DestroyTypetag(TutTypetag* tag);

#endif
