#ifndef TUT_LEXERCONTEXT_H
#define TUT_LEXERCONTEXT_H

#include "tut_symbols.h"

typedef struct
{
	const char* filename;
	int line;
	const char* lineStart;
	const char* current;
} TutLexerContext;

#endif
