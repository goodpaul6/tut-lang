#include <string.h>
#include <stdlib.h>
#include <assert.h>

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
		}
		else
			putc(str[i], stdout);
	}
	
	return 0;
}

static uint16_t ExtStrlen(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	const char* str = args[0].sv;
	Tut_PushInt(vm, (int)strlen(str));

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

void TutStdExt_BindAll(TutModule* module, TutVM* vm)
{
	Tut_BindExternFindIndex(module, vm, "printf", ExtPrintf);
	Tut_BindExternFindIndex(module, vm, "strlen", ExtStrlen);
	Tut_BindExternFindIndex(module, vm, "malloc", ExtMalloc);
	Tut_BindExternFindIndex(module, vm, "memcpy", ExtMemcpy);
	Tut_BindExternFindIndex(module, vm, "radd", ExtRadd);
	Tut_BindExternFindIndex(module, vm, "free", ExtFree);
}