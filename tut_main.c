#include "tut_lexer.h"
#include "tut_util.h"

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		FILE* file = fopen(argv[1], "rb");
		if(!file)
			Tut_ErrorExit("Failed to open file '%s' for reading.\n", argv[1]);
		
		TutLexer lexer;
		Tut_InitLexerFromFile(&lexer, file);
		
		fclose(file);
		
		while(lexer.curTok != TUT_TOK_EOF)
		{
			Tut_GetToken(&lexer);
			printf("%s\n", Tut_TokenRepr(lexer.curTok));
		}
		
		Tut_DestroyLexer(&lexer);
		return TUT_SUCCESS;
	}
	
	fprintf(stderr, "Usage:\n%s (path/to/file).\n", argv[0]);
	return TUT_FAILURE;
}
