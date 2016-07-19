#ifndef TUT_EXPR_H
#define TUT_EXPR_H

#include "tut_list.h"

typedef enum
{
	TUT_EXPR_BLOCK,
	
	TUT_EXPR_NUM,
	TUT_EXPR_STR,
	TUT_EXPR_VAR,
	
	TUT_EXPR_BIN,
	TUT_EXPR_PAREN,
	
	TUT_EXPR_FUNC_PROTO,
	TUT_EXPR_FUNC,
	
	TUT_EXPR_RETURN,
	TUT_EXPR_IF,
	TUT_EXPR_WHILE
} TutExprType;

typedef struct
{
	TutExprType type;
	
	union
	{
		TutList blockList;
		
	};
} TutExpr;

#endif
