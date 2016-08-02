#include <string.h>

#include "tut_stdext.h"
#include "tut_compiler.h"

TutBool TutStdExt_Printf(TutVM* vm, uint8_t nargs)
{
	const char* str = Tut_PopCString(vm);
	size_t length = strlen(str);

	for (size_t i = 0; i < length; ++i)
	{
		if (str[i] == '%')
		{
			++i;
			if (str[i] == 'i')
			{
				int32_t value = Tut_PopInt(vm);
				printf("%i", value);
			}
			else if (str[i] == 'f')
			{
				float value = Tut_PopFloat(vm);
				printf("%f", value);
			}
			else if (str[i] == 's')
			{
				const char* str = Tut_PopCString(vm);
				printf("%s", str);
			}
		}
		else
			putc(str[i], stdout);
	}
	
	return TUT_FALSE;
}

void TutStdExt_BindAll(TutModule* module, TutVM* vm)
{
	const TutExternDef lib[] = 
	{
		{ "printf", TutStdExt_Printf } 
	};
	Tut_BindLibrary(module, vm, sizeof(lib) / sizeof(lib[0]), lib);
}