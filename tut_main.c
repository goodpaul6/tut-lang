#include "tut_module.h"
#include "tut_expr.h"

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		TutModule module;
		
		Tut_InitModuleFromFile(&module, argv[1]);
		Tut_DestroyModule(&module);
		
		return TUT_SUCCESS;
	}
	
	fprintf(stderr, "Usage:\n%s (path/to/file).\n", argv[0]);
	return TUT_FAILURE;
}
