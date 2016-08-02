#include <assert.h>

#include "tut_expr.h"
#include "tut_opcodes.h"
#include "tut_codegen.h"
#include "tut_compiler.h"
#include "tut_stdext.h"

static void TestVM()
{
	TutVM vm;

	Tut_InitVM(&vm);

	int32_t patchLoc = Tut_EmitGoto(&vm, TUT_FALSE, 0);
	
	Tut_EmitFunctionEntryPoint(&vm);
	Tut_EmitGet(&vm, TUT_FALSE, -2);
	Tut_EmitGet(&vm, TUT_FALSE, -1);
	Tut_EmitOp(&vm, TUT_OP_SUBI);
	Tut_EmitOp(&vm, TUT_OP_RETVAL);
	
	Tut_PatchGoto(&vm, patchLoc, vm.codeSize);

	Tut_EmitPushInt(&vm, 100);
	Tut_EmitPushInt(&vm, 200);
	Tut_EmitCall(&vm, TUT_FALSE, 0, 2);

	Tut_EmitOp(&vm, TUT_OP_HALT);

	vm.pc = 0;
	while(vm.pc >= 0)
		Tut_ExecuteCycle(&vm, TUT_FALSE);

	assert(vm.stack[0].iv == -100);

	Tut_DestroyVM(&vm);
}

static void TestCompiler(const char* filename)
{
	TutVM vm;
	Tut_InitVM(&vm);

	TutModule module;

	Tut_InitModuleFromFile(&module, filename);
	Tut_CompileModule(&module, &vm);
	
	TutStdExt_BindAll(&module, &vm);

	Tut_DestroyModule(&module);

	vm.pc = 0;
	while (vm.pc >= 0)
		Tut_ExecuteCycle(&vm, TUT_FALSE);
}

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		//TestVM();
		TestCompiler(argv[1]);
		getchar();

		return TUT_SUCCESS;
	}
	
	fprintf(stderr, "Usage:\n%s (path/to/file).\n", argv[0]);
	return TUT_FAILURE;
}
