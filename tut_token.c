#include <assert.h>

#include "tut_token.h"

static const char* Names[TUT_TOK_COUNT] = 
{
	"number",
	"string",
	"identifier",
	
	"var",
	"func",
	"return",
	"if",
	"else",
	"while",
	
	"(",
	")",
	"[",
	"]",
	"{",
	"}",
	
	",",
	
	"+",
	"-",
	"*",
	"/",
	"=",
	
	"EOF",
	
	"ERROR"
};

const char* Tut_TokenRepr(TutToken token)
{
	assert(token >= 0 && token < TUT_TOK_COUNT);
	return Names[(int)token];
}
