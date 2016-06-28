#ifndef TUT_LEXER_H
#define TUT_LEXER_H

#define TUT_MAX_LEXEME_LENGTH 256

#include <stdio.h>

#include "tut_token.h"

typedef struct
{
	char* source;
	
	const char* lineStart;
	const char* current;

	int last;
	char lexeme[TUT_MAX_LEXEME_LENGTH];
	double number;
	
	TutToken curTok;
} TutLexer;

void Tut_InitLexer(TutLexer* lexer, const char* source);
void Tut_InitLexerFromFile(TutLexer* lexer, FILE* file);

TutToken Tut_GetToken(TutLexer* lexer);

void Tut_DestroyLexer(TutLexer* lexer);

#endif
