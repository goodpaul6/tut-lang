#include <assert.h>

#include "tut_token.h"

static const char* Names[TUT_TOK_COUNT] = 
{
	"true",
	"false",

	"integer literal",
	"float literal",
	"string literal",
	"identifier",
	
	"cast",

	"var",
	"func",
	"extern",
	"return",
	"if",
	"else",
	"while",

	"struct",
	
	"(",
	")",
	"[",
	"]",
	"{",
	"}",
	
	":",
	";",
	",",
	".",
	"->",
	"...",
	
	"+",
	"-",
	"*",
	"/",

	"<",
	">",
	"<=",
	">=",

	"&",
	"|",

	"&&",
	"||",
	"!"

	"=",

	"!=",
	"==",
	
	"EOF",
	
	"ERROR"
};

const char* Tut_TokenRepr(TutToken token)
{
	assert(token >= 0 && token < TUT_TOK_COUNT);
	return Names[(int)token];
}
