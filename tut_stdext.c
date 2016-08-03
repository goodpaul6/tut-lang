#include <string.h>

#include "tut_stdext.h"
#include "tut_compiler.h"

uint16_t TutStdExt_Printf(TutVM* vm, const TutObject* args, uint16_t nargs)
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

uint16_t TutStdExt_Strlen(TutVM* vm, const TutObject* args, uint16_t nargs)
{
	const char* str = args[0].sv;
	Tut_PushInt(vm, (int)strlen(str));

	return 1;
}

void TutStdExt_BindAll(TutModule* module, TutVM* vm)
{
	const TutExternDef lib[] = 
	{
		{ "printf", TutStdExt_Printf },
		{ "strlen", TutStdExt_Strlen }
	};
	Tut_BindLibrary(module, vm, sizeof(lib) / sizeof(lib[0]), lib);
}