#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "tut_stdext.h"
#include "tut_compiler.h"

static uint16_t ExtPrintf(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	const char* str = args[0].sv;
	int argIndex = 1;
	size_t length = strlen(str);

	for (size_t i = 0; i < length; ++i)
	{
		if (str[i] == '%')
		{
			++i;
			if (str[i] == 'i')
			{
				int32_t value = args[argIndex++].iv;
				printf("%i", value);
			}
			else if (str[i] == 'f')
			{
				float value = args[argIndex++].fv;
				printf("%f", value);
			}
			else if (str[i] == 's')
			{
				const char* str = args[argIndex++].sv;
				printf("%s", str);
			}
			else if (str[i] == '.')
			{
				const TutObject* obj = &args[argIndex++];

				switch (obj->type)
				{
					case TUT_OBJECT_BOOL: printf("%s", obj->bv ? "true" : "false"); break;
					case TUT_OBJECT_INT: printf("%i", obj->iv); break;
					case TUT_OBJECT_FLOAT: printf("%f", obj->fv); break;
					case TUT_OBJECT_STR: case TUT_OBJECT_CSTR: printf("%s", obj->sv); break;
					case TUT_OBJECT_FUNC: 
					{
						if (obj->func.isExtern)
							printf("extern %s [%i]", TUT_ARRAY_GET_VALUE(&vm->externNames, obj->func.index, const char*), obj->func.index);
						else
							printf("function [%i]", obj->func.index);
					} break;
					case TUT_OBJECT_REF: printf("ref %" PRIdPTR, (uintptr_t)obj->ref); break;
					case TUT_OBJECT_PTR: printf("ptr %" PRIdPTR, (uintptr_t)obj->ref); break;
				}
			}
		}
		else
			putc(str[i], stdout);
	}
	
	return 0;
}

static uint16_t ExtStrlen(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	Tut_PushInt(vm, (int32_t)strlen(args[0].sv));

	return 1;
}

static uint16_t ExtMalloc(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	assert(args[0].iv >= 0);
	void* mem = Tut_Malloc(args[0].iv);

	Tut_PushRef(vm, mem);

	return 1;
}

static uint16_t ExtMemcpy(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	assert(args[0].ref && args[1].ref);
	
	size_t sizeInBytes = (size_t)args[2].iv;

	void* ref = memcpy(args[0].ref, args[1].ref, sizeInBytes);
	Tut_PushRef(vm, ref);

	return 1;
}

static uint16_t ExtRadd(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	assert(args[0].ref);

	Tut_PushRef(vm, (void*)((intptr_t)args[0].ref + args[1].iv));

	return 1;
}

static uint16_t ExtFree(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	Tut_Free(args[0].ref);
	return 0;
}

static uint16_t ExtTostr(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	Tut_PushString(vm, Tut_Strdup(args[0].sv));
	return 1;
}

static uint16_t ExtSubstr(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	const char* str = args[0].sv;
	int start = args[1].iv;
	int end = args[2].iv;
	int len = end - start;

	char* buf = Tut_Malloc(end - start + 1);
	buf[len] = '\0';

	memcpy(buf, str + start, len);

	Tut_PushStringNoCopy(vm, buf);
	return 1;
}

static uint16_t ExtFreestr(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	char* str = args[0].sv;
	assert(str);

	Tut_Free(str);
	return 0;
}

static uint16_t ExtGettype(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	Tut_PushInt(vm, args[0].type);
	return 1;
}

void TutStdExt_BindAll(TutModule* module, TutVM* vm)
{
	Tut_BindExternFindIndex(module, vm, "gettype", ExtGettype);
	Tut_BindExternFindIndex(module, vm, "printf", ExtPrintf);
	Tut_BindExternFindIndex(module, vm, "strlen", ExtStrlen);
	Tut_BindExternFindIndex(module, vm, "malloc", ExtMalloc);
	Tut_BindExternFindIndex(module, vm, "memcpy", ExtMemcpy);
	Tut_BindExternFindIndex(module, vm, "radd", ExtRadd);
	Tut_BindExternFindIndex(module, vm, "free", ExtFree);
	Tut_BindExternFindIndex(module, vm, "tostr", ExtTostr);
	Tut_BindExternFindIndex(module, vm, "substr", ExtSubstr);
	Tut_BindExternFindIndex(module, vm, "freestr", ExtFreestr);
}