#ifndef TUT_EXPR_H
#define TUT_EXPR_H

#include "tut_symbols.h"
#include "tut_lexercontext.h"
#include "tut_array.h"
#include "tut_token.h"

typedef enum
{
	TUT_EXPR_BLOCK,

	TUT_EXPR_TRUE,
	TUT_EXPR_FALSE,
	TUT_EXPR_NULL,

	TUT_EXPR_INT,
	TUT_EXPR_FLOAT,
	TUT_EXPR_STR,

	TUT_EXPR_IDENT,
	TUT_EXPR_VAR,

	TUT_EXPR_UNARY,
	TUT_EXPR_BIN,
	TUT_EXPR_PAREN,

	TUT_EXPR_ARROW,
	TUT_EXPR_DOT,

	TUT_EXPR_CALL,
	
	TUT_EXPR_FUNC,
	
	TUT_EXPR_RETURN,
	TUT_EXPR_IF,
	TUT_EXPR_WHILE,

	TUT_EXPR_CAST,
	TUT_EXPR_SIZEOF,

	TUT_EXPR_STRUCT_DEF,
} TutExprType;

typedef struct TutExpr
{
	TutExprType type;
	TutTypetag* typetag;
	TutLexerContext context;

	union
	{
		TutList blockList;
		
		int intVal;
		float floatVal;
		char* string;
		
		// Used by both EXPR_VAR and EXPR_IDENT
		struct
		{
			char* name;
			TutVarDecl* decl;
			// This is for ResolveSymbols to cache the function
			// once so it doesn't have to be looked up again in 
			// ResolveTypes and CompileValue
			TutFuncDecl* funcDecl;
			// Same as above (cache typetag so it can be used with
			// sizeof etc)
			TutTypetag* typetag;
		} varx;
		
		struct
		{
			TutToken op;
			struct TutExpr* value;
		} unaryx;

		struct
		{
			struct TutExpr* lhs;
			struct TutExpr* rhs;
			int op;
		} binx;
		
		struct TutExpr* parenExpr;
		
		// Used by both EXPR_DOT and EXPR_ARROW
		struct
		{
			struct TutExpr* value;
			char* memberName;
		} dotx;

		struct
		{
			struct TutExpr* func;
			TutList args;
		} callx;
		
		struct
		{
			TutFuncDecl* decl;
			struct TutExpr* body;
		} funcx;
		
		struct
		{
			TutFuncDecl* parent;
			struct TutExpr* value;
		} retx;
		
		struct
		{
			struct TutExpr* cond;
			struct TutExpr* body;
			struct TutExpr* alt;
		} ifx;
		
		struct
		{
			struct TutExpr* cond;
			struct TutExpr* body;
		} whilex;

		struct
		{
			struct TutExpr* value;
			TutTypetag* typetag;
		} castx;

		struct
		{
			struct TutExpr* value;
		} sizeofx;
		
		struct
		{
			struct TutTypetag* typetag;
		} structx;
	};
} TutExpr;

TutExpr* Tut_CreateExpr(TutExprType type, const TutLexerContext* context);
void Tut_FlattenExpr(TutExpr* exp, TutArray* into);
void Tut_DestroyExpr(TutExpr* expr);

#endif
