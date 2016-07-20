#ifndef TUT_LEXERCONTEXT_H
#define TUT_LEXERCONTEXT_H

typedef struct
{
	const char* filename;
	int line;
	const char* lineStart;
	const char* current;
} TutLexerContext;

#endif
