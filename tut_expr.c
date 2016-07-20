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

void Tut_DestroyExpr(TutExpr* expr)
{
	// TODO: Implement this
}
