#ifndef TUT_LEXER_H
#define TUT_LEXER_H

#define TUT_MAX_LEXEME_LENGTH 256

#include <stdio.h>

#include "tut_token.h"
#include "tut_lexercontext.h"

typedef struct
{
	char* source;							// Stores the source code being lexed
	
	TutLexerContext context;
	
	int last;								// Last character read from stream
	char lexeme[TUT_MAX_LEXEME_LENGTH];		// Stores the token as a string (ex. if the token was
											// a string "hello world", this would contain
											// that string)
											
	double number;							// Stores the numerical value of the token if the token is
											// TUT_TOK_NUMBER

	TutToken curTok;						// Last token (set by Tut_GetToken)
} TutLexer;

void Tut_InitLexer(TutLexer* lexer, const char* source);
void Tut_InitLexerFromFile(TutLexer* lexer, FILE* file);

TutToken Tut_GetToken(TutLexer* lexer);

void Tut_DestroyLexer(TutLexer* lexer);

#endif
