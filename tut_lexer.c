#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "tut_lexer.h"
#include "tut_util.h"

static void InitLexer(TutLexer* lexer, char* source)
{
	lexer->source = source;	
	
	lexer->context.filename = NULL;
	lexer->context.line = 1;
	lexer->context.lineStart = source;
	lexer->context.current = source;
	
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
	if(*lexer->context.current)
	{
		int c = (int)(*lexer->context.current);
		lexer->context.current++;
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
		if(prev == '\n')
		{ 
			++lexer->context.line;
			lexer->context.lineStart = lexer->context.current;
		}
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

		if (strcmp(lexer->lexeme, "true") == 0) return TUT_TOK_TRUE;
		if (strcmp(lexer->lexeme, "false") == 0) return TUT_TOK_FALSE;
		if (strcmp(lexer->lexeme, "if") == 0) return TUT_TOK_IF;
		if (strcmp(lexer->lexeme, "else") == 0) return TUT_TOK_ELSE;
		if (strcmp(lexer->lexeme, "while") == 0) return TUT_TOK_WHILE;
		if (strcmp(lexer->lexeme, "var") == 0) return TUT_TOK_VAR;
		if (strcmp(lexer->lexeme, "func") == 0) return TUT_TOK_FUNC;
		if (strcmp(lexer->lexeme, "return") == 0) return TUT_TOK_RETURN;
		if (strcmp(lexer->lexeme, "extern") == 0) return TUT_TOK_EXTERN;
		if (strcmp(lexer->lexeme, "struct") == 0) return TUT_TOK_STRUCT;
		if (strcmp(lexer->lexeme, "cast") == 0) return TUT_TOK_CAST;
		if (strcmp(lexer->lexeme, "import") == 0) return TUT_TOK_IMPORT;
		if (strcmp(lexer->lexeme, "module") == 0) return TUT_TOK_MODULE;
		if (strcmp(lexer->lexeme, "null") == 0) return TUT_TOK_NULL;

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
		lexer->number = strtod(lexer->lexeme, NULL);
		
		if(hasRadix)
			return TUT_TOK_FLOAT;
		return TUT_TOK_INT;
	}
	
	if(lexer->last == '"')
	{
		int i = 0;
		lexer->last = GetChar(lexer);
		
		while(lexer->last != '"')
		{
			if (i < TUT_MAX_LEXEME_LENGTH - 1)
			{
				if (lexer->last == '\\')
				{
					lexer->last = GetChar(lexer);
					
					switch (lexer->last)
					{
						case 'n': lexer->last = '\n'; break;
						case 't': lexer->last = '\t'; break;
						case 'r': lexer->last = '\r'; break;
						case 'b': lexer->last = '\b'; break;
						case '0': lexer->last = '\0'; break;
						case '\\': lexer->last = '\\'; break;
						default:
							Tut_ErrorExit("Invalid escape sequence '\%c'\n", lexer->last);
							return TUT_TOK_ERROR;
							break;
					}
				}

				lexer->lexeme[i++] = lexer->last;
			}
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
	
	if(lexer->last == ':')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_COLON;
	}
	
	if(lexer->last == ';')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_SEMICOLON;
	}
	
	if(lexer->last == ',')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_COMMA;
	}
	
	if (lexer->last == '.')
	{
		lexer->last = GetChar(lexer);
		
		if (lexer->last == '.')
		{
			lexer->last = GetChar(lexer);
			if (lexer->last == '.')
			{
				lexer->last = GetChar(lexer);
				return TUT_TOK_ELLIPSIS;
			}

			Tut_ErrorExit("Invalid token '..'\n");
			return TUT_TOK_ERROR;
		}

		return TUT_TOK_DOT;
	}

	if(lexer->last == '+')
	{
		lexer->last = GetChar(lexer);
		return TUT_TOK_PLUS;
	}
	
	if(lexer->last == '-')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '>')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_ARROW;
		}

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
	
	if (lexer->last == '<')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '=')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_LTE;
		}

		return TUT_TOK_LT;
	}

	if (lexer->last == '>')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '=')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_GTE;
		}

		return TUT_TOK_GT;
	}

	if (lexer->last == '&')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '&')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_LAND;
		}

		return TUT_TOK_AND;
	}

	if (lexer->last == '|')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '|')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_LOR;
		}

		return TUT_TOK_OR;
	}

	if(lexer->last == '=')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '=')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_EQUALS;
		}

		return TUT_TOK_ASSIGN;
	}

	if (lexer->last == '!')
	{
		lexer->last = GetChar(lexer);
		if (lexer->last == '=')
		{
			lexer->last = GetChar(lexer);
			return TUT_TOK_NEQUALS;
		}

		return TUT_TOK_LNOT;
	}
	
	if(lexer->last == EOF)
		return TUT_TOK_EOF;
		
	Tut_ErrorExit("(%s, %d): Unexpected character '%c'.\n", lexer->context.filename, lexer->context.line, lexer->last);
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
