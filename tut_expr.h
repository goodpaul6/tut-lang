#ifndef TUT_EXPR_H
#define TUT_EXPR_H

#include "tut_symbols.h"
#include "tut_lexercontext.h"

typedef enum
{
	TUT_EXPR_BLOCK,
	
	TUT_EXPR_INT,
	TUT_EXPR_FLOAT,
	TUT_EXPR_STR,
	TUT_EXPR_VAR,
	
	TUT_EXPR_BIN,
	TUT_EXPR_PAREN,
	TUT_EXPR_CALL,
	
	TUT_EXPR_FUNC,
	
	TUT_EXPR_RETURN,
	TUT_EXPR_IF,
	TUT_EXPR_WHILE
} TutExprType;

typedef struct TutExpr
{
	TutExprType type;
	TutLexerContext context;
	TutTypetag* typetag;
		
	union
	{
		TutList blockList;
		
		int intVal;
		float floatVal;
		char* string;
	
		struct
		{
			char* name;
			TutVarDecl* decl;
		} varx;
		
		struct
		{
			struct TutExpr* lhs;
			struct TutExpr* rhs;
			int op;
		} binx;
		
		struct TutExpr* parenExpr;
		
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
	};
} TutExpr;

TutExpr* Tut_CreateExpr(TutExprType type, const TutLexerContext* context);
void Tut_DestroyExpr(TutExpr* expr);

#endif
