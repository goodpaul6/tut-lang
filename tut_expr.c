#include <assert.h>

#include "tut_util.h"
#include "tut_expr.h"

TutExpr* Tut_CreateExpr(TutExprType type, const TutLexerContext* context)
{
	TutExpr* exp = Tut_Malloc(sizeof(TutExpr));
	
	exp->type = type;
	exp->typetag = NULL;
	exp->context = *context;

	return exp;
}

void Tut_FlattenExpr(TutExpr* exp, TutArray* into)
{
	assert(exp);

	Tut_ArrayPush(into, &exp);

	switch (exp->type)
	{
		case TUT_EXPR_INT:
		case TUT_EXPR_FLOAT:
		case TUT_EXPR_STR:
		case TUT_EXPR_STRUCT_DEF:
		case TUT_EXPR_VAR:
		case TUT_EXPR_IDENT:
		case TUT_EXPR_EXTERN:
		{
			// No sub expressions
		} break;

		case TUT_EXPR_ARROW:
		case TUT_EXPR_DOT:
		{
			Tut_FlattenExpr(exp->dotx.value, into);
		} break;

		case TUT_EXPR_UNARY:
		{
			Tut_FlattenExpr(exp->unaryx.value, into);
		} break;

		case TUT_EXPR_BIN:
		{
			Tut_FlattenExpr(exp->binx.lhs, into);
			Tut_FlattenExpr(exp->binx.rhs, into);
		} break;

		case TUT_EXPR_PAREN:
		{
			Tut_FlattenExpr(exp->parenExpr, into);
		} break;

		case TUT_EXPR_CALL:
		{
			Tut_FlattenExpr(exp->callx.func, into);
			TUT_LIST_EACH(node, exp->callx.args)
				Tut_FlattenExpr(node->value, into);
		} break;

		case TUT_EXPR_BLOCK:
		{
			TUT_LIST_EACH(node, exp->blockList)
				Tut_FlattenExpr(node->value, into);
		} break;

		case TUT_EXPR_RETURN:
		{
			Tut_FlattenExpr(exp->retx.value, into);
		} break;

		case TUT_EXPR_IF:
		{
			Tut_FlattenExpr(exp->ifx.cond, into);
			Tut_FlattenExpr(exp->ifx.body, into);
			if (exp->ifx.alt)
				Tut_FlattenExpr(exp->ifx.alt, into);
		} break;

		case TUT_EXPR_WHILE:
		{
			Tut_FlattenExpr(exp->whilex.cond, into);
			Tut_FlattenExpr(exp->whilex.body, into);
		} break;

		case TUT_EXPR_FUNC:
		{
			Tut_FlattenExpr(exp->funcx.body, into);
		} break;
	}
}

void Tut_DestroyExpr(TutExpr* expr)
{
	// TODO: Implement this
}
