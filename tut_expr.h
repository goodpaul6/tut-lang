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

	TUT_EXPR_IDENT,
	TUT_EXPR_VAR,

	TUT_EXPR_UNARY,
	TUT_EXPR_BIN,
	TUT_EXPR_PAREN,

	TUT_EXPR_DOT,
	TUT_EXPR_CALL,
	
	TUT_EXPR_FUNC,
	TUT_EXPR_EXTERN,
	
	TUT_EXPR_RETURN,
	TUT_EXPR_IF,
	TUT_EXPR_WHILE,

	TUT_EXPR_STRUCT_DEF,
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
		
		// Used by both EXPR_VAR and EXPR_IDENT
		struct
		{
			char* name;
			TutVarDecl* decl;
		} varx;
		
		struct
		{
			int op;
			struct TutExpr* value;
		} unaryx;

		struct
		{
			struct TutExpr* lhs;
			struct TutExpr* rhs;
			int op;
		} binx;
		
		struct TutExpr* parenExpr;
		
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
			TutFuncDecl* decl;
		} externx;

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
			struct TutTypetag* typetag;
		} structx;
	};
} TutExpr;

TutExpr* Tut_CreateExpr(TutExprType type, const TutLexerContext* context);
void Tut_DestroyExpr(TutExpr* expr);

#endif
