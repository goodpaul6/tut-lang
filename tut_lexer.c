#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "tut_lexer.h"
#include "tut_util.h"

static void InitLexer(TutLexer* lexer, char* source)
{
	lexer->source = source;	
	lexer->lineStart = lexer->source;
	lexer->current = lexer->source;
	
	lexer->curTok = TUT_TOK_ERROR;

	lexer->last = ' ';
	lexer->lexeme[0] = '\0';
	lexer->number = 0;
}

void Tut_InitLexer(TutLexer* lexer, const char* source)
{
	size_t sourceLength = strlen(source);
	char* str = Tut_Malloc(sourceLength + 1);
	strcpy(str, source);

	InitLexer(lexer, str);
}

void Tut_InitLexerFromFile(TutLexer* lexer, FILE* file)
{
	assert(file);
	
	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	rewind(file);
	
	char* str = Tut_Malloc(length + 1);
	fread(str, 1, length, file);
	str[length] = '\0';
	
	InitLexer(lexer, str);
}

static int GetChar(TutLexer* lexer)
{
	if(*lexer->current)
	{
		int c = (int)(*lexer->current);
		lexer->current++;
		return c;
	}
	
	return EOF;
}

static TutToken GetToken(TutLexer* lexer)
{
	while(isspace(lexer->last))
	{
		int prev = lexer->last;
		lexer->last = GetChar(lexer);
		if(prev == '\n') lexer->lineStart = lexer->current;
	}
	
	if(isalpha(lexer->last) || lexer->last == '_')
	{
		int i = 0;
		while(isalnum(lexer->last) || lexer->last == '_')
		{
			if(i < TUT_MAX_LEXEME_LENGTH - 1)
				lexer->lexeme[i++] = lexer->last;
			else
				Tut_ErrorExit("Identifier exceeded maximum lexeme length.\n");
			lexer->last = GetChar(lexer);
		}
		
		lexer->lexeme[i] = '\0';
		
		if(strcmp(lexer->lexeme, "if") == 0) return TUT_TOK_IF;
		if(strcmp(lexer->lexeme, "else") == 0) return TUT_TOK_ELSE;
		if(strcmp(lexer->lexeme, "while") == 0) return TUT_TOK_WHILE;
		if(strcmp(lexer->lexeme, "var") == 0) return TUT_TOK_VAR;
		if(strcmp(lexer->lexeme, "func") == 0) return TUT_TOK_FUNC;
		if(strcmp(lexer->lexeme, "return") == 0) return TUT_TOK_RETURN;
		
		return TUT_TOK_IDENT;
	}
	
	if(isdigit(lexer->last))
	{
		int i = 0;
		TutBool hasRadix = TUT_FALSE;
		
		while(isdigit(lexer->last) || lexer->last == '.')
		{
			if(hasRadix && lexer->last == '.')
				Tut_ErrorExit("Number token contains multiple radices ('.').\n");
			
			if(i < TUT_MAX_LEXEME_LENGTH - 1)
				lexer->lexeme[i++] = lexer->last;
			else
				Tut_ErrorExit("Number exceeded maximum lexeme length.\n");
			
			if(lexer->last == '.')
				hasRadix = TUT_TRUE;
			lexer->last = GetChar(lexer);
		}
		
		lexer->lexeme[i] = '\0';
		return TUT_TOK_NUMBER;
	}
	
	if(lexer->last == '"')
	{
		int i = 0;
		lexer->last = GetChar(lexer);
		
		while(lexer->last != '"')
		{
			if(i < TUT_MAX_LEXEME_LENGTH - 1)
				lexer->lexeme[i++] = lexer->last;
			else
				Tut_ErrorExit("String exceeded maximum lexeme length.\n");
			
			lexer->last = GetChar(lexer);
		}
		
		lexer->lexeme[i] = '\0';
		lexer->last = GetChar(lexer);
	
		return TUT_TOK_STRING;
	}
	
	if(lexer->last == '(')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_OPENPAREN;
	}
	
	if(lexer->last == ')')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_CLOSEPAREN;
	}
	
	if(lexer->last == '[')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_OPENSQUARE;
	}
	
	if(lexer->last == ']')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_CLOSESQUARE;
	}
	
	if(lexer->last == '{')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_OPENCURLY;
	}
	
	if(lexer->last == '}')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_CLOSECURLY;
	}
	
	if(lexer->last == ',')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_COMMA;
	}
	
	if(lexer->last == '+')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_PLUS;
	}
	
	if(lexer->last == '-')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_MINUS;
	}
	
	if(lexer->last == '*')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_MUL;
	}
	
	if(lexer->last == '/')
	{
		lexer->last = GetChar(lexer);
		if(lexer->last == '/')
		{
			while(lexer->last != '\n' && lexer->last != EOF)
				lexer->last = GetChar(lexer);
			
			return GetToken(lexer);
		}
		
		return TUT_TOK_DIV;
	}
	
	if(lexer->last == '=')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_ASSIGN;
	}
	
	if(lexer->last == EOF)
		return TUT_TOK_EOF;
		
	Tut_ErrorExit("Unexpected character '%c'.\n", lexer->last);
	return TUT_TOK_ERROR;
}

TutToken Tut_GetToken(TutLexer* lexer)
{
	lexer->curTok = GetToken(lexer);
	return lexer->curTok;
}

void Tut_DestroyLexer(TutLexer* lexer)
{
	Tut_Free(lexer->source);
}