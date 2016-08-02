#include <stdarg.h>
#include <stdlib.h>

#include "tut_parser.h"
#include "tut_expr.h"

static void ParseError(TutModule* module, const char* format, ...)
{
	fprintf(stderr, "Error (%s, %i): ", module->name, module->lexer.context.line);
	
	va_list args;
	va_start(args, format);
	
	vfprintf(stderr, format, args);
	
	va_end(args);
	
	exit(1);
}

static void ExpectToken(TutModule* module, TutToken token)
{
	if (module->lexer.curTok != token)
		ParseError(module, "Expected token '%s' but received '%s'.\n", Tut_TokenRepr(token), Tut_TokenRepr(module->lexer.curTok));
}

static void EatToken(TutModule* module, TutToken token)
{
	ExpectToken(module, token);
	Tut_GetToken(&module->lexer);
}

static TutExpr* ParseExpr(TutModule* module);

static TutTypetag* ParseType(TutModule* module)
{
	ExpectToken(module, TUT_TOK_IDENT);

	// Attempt to create a primitive type tag (i.e bool, int, cstr, etc)
	TutTypetag* tag = Tut_CreatePrimitiveTypetag(module->lexer.lexeme);
	if(!tag)
		tag = Tut_RegisterType(&module->symbolTable, module->lexer.lexeme);
	
	Tut_GetToken(&module->lexer);
	return tag;
}

static TutExpr* ParseInt(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_INT, &module->lexer.context);
	exp->intVal = (int)module->lexer.number;
	
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseFloat(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_FLOAT, &module->lexer.context);
	exp->floatVal = (float)module->lexer.number;
	
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseString(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_STR, &module->lexer.context);
	exp->string = Tut_Strdup(module->lexer.lexeme);

	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseVar(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_VAR, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	ExpectToken(module, TUT_TOK_IDENT);
	
	exp->varx.name = Tut_Strdup(module->lexer.lexeme);
	Tut_GetToken(&module->lexer);
	
	EatToken(module, TUT_TOK_COLON);

	exp->varx.decl = Tut_DeclareVariable(&module->symbolTable, exp->varx.name, ParseType(module));
	
	return exp;
}

static TutExpr* ParseIdent(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_IDENT, &module->lexer.context);
				
	exp->varx.name = Tut_Strdup(module->lexer.lexeme);
	exp->varx.decl = Tut_GetVarDecl(&module->symbolTable, module->lexer.lexeme, -1);
	
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseFunc(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_FUNC, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	ExpectToken(module, TUT_TOK_IDENT);

	exp->funcx.decl = Tut_DeclareFunction(&module->symbolTable, module->lexer.lexeme);
	Tut_GetToken(&module->lexer);
	
	Tut_PushCurFuncDecl(&module->symbolTable, exp->funcx.decl);
	
	EatToken(module, TUT_TOK_OPENPAREN);

	while(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
	{
		if (module->lexer.curTok == TUT_TOK_ELLIPSIS)
			ParseError(module, "Only extern functions can have variable arguments.\n"); // TODO: Fix this
		else
			ExpectToken(module, TUT_TOK_IDENT);

		char* name = Tut_Strdup(module->lexer.lexeme);
		Tut_GetToken(&module->lexer);
		
		EatToken(module, TUT_TOK_COLON);
		
		TutTypetag* tag = ParseType(module);
		
		Tut_DeclareArgument(&module->symbolTable, name, tag);
		
		if(module->lexer.curTok == TUT_TOK_COMMA)
			Tut_GetToken(&module->lexer);
		else if(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
			ParseError(module, "Expected ')' or ',' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	}
	Tut_GetToken(&module->lexer);	
	
	EatToken(module, TUT_TOK_COLON);

	exp->funcx.decl->returnType = ParseType(module);
	exp->funcx.body = ParseExpr(module);

	Tut_PopCurFuncDecl(&module->symbolTable);
	
	return exp;
}

static TutExpr* ParseExtern(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_EXTERN, &module->lexer.context);
	Tut_GetToken(&module->lexer);

	ExpectToken(module, TUT_TOK_IDENT);

	exp->externx.decl = Tut_DeclareExtern(&module->symbolTable, module->lexer.lexeme);
	Tut_GetToken(&module->lexer);

	Tut_PushCurFuncDecl(&module->symbolTable, exp->externx.decl);

	EatToken(module, TUT_TOK_OPENPAREN);

	while (module->lexer.curTok != TUT_TOK_CLOSEPAREN)
	{
		if (module->lexer.curTok == TUT_TOK_ELLIPSIS)
		{
			exp->funcx.decl->hasVarargs = TUT_TRUE;
			Tut_GetToken(&module->lexer);

			if (module->lexer.curTok != TUT_TOK_CLOSEPAREN)
				ParseError(module, "Expected ')' after '...' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
			break;
		}
		else
			ExpectToken(module, TUT_TOK_IDENT);

		char* name = Tut_Strdup(module->lexer.lexeme);
		Tut_GetToken(&module->lexer);

		EatToken(module, TUT_TOK_COLON);

		TutTypetag* tag = ParseType(module);

		Tut_DeclareArgument(&module->symbolTable, name, tag);

		if (module->lexer.curTok == TUT_TOK_COMMA)
			Tut_GetToken(&module->lexer);
		else if (module->lexer.curTok != TUT_TOK_CLOSEPAREN)
			ParseError(module, "Expected ')' or ',' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	}
	Tut_GetToken(&module->lexer);

	EatToken(module, TUT_TOK_COLON);

	exp->externx.decl->returnType = ParseType(module);

	Tut_PopCurFuncDecl(&module->symbolTable);

	return exp;
}

static TutExpr* ParseReturn(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_RETURN, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	if (!module->symbolTable.curFunc)
		ParseError(module, "Return statement not inside function.\n");

	exp->retx.parent = module->symbolTable.curFunc;
	
	if(module->lexer.curTok == TUT_TOK_SEMICOLON)
	{
		exp->retx.value = NULL;
		return exp;
	}
	
	exp->retx.value = ParseExpr(module);
	
	return exp;
}

static TutExpr* ParseParen(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_PAREN, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	exp->parenExpr = ParseExpr(module);
	
	if(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
		ParseError(module, "Expected matching ')' after previous '(' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	Tut_GetToken(&module->lexer);
	
	return exp;	
}

static TutExpr* ParseBlock(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_BLOCK, &module->lexer.context);
	Tut_InitList(&exp->blockList);
	
	Tut_GetToken(&module->lexer);

	++module->symbolTable.curScope;

	while(module->lexer.curTok != TUT_TOK_CLOSECURLY)
	{
		TutExpr* bodyExp = ParseExpr(module);
		if(module->lexer.curTok == TUT_TOK_SEMICOLON)
			Tut_GetToken(&module->lexer);
		
		Tut_ListAppend(&exp->blockList, bodyExp);
	}

	--module->symbolTable.curScope;

	Tut_GetToken(&module->lexer);

	return exp;
}

static TutExpr* ParseIf(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_IF, &module->lexer.context);
	Tut_GetToken(&module->lexer);

	exp->ifx.cond = ParseExpr(module);
	exp->ifx.body = ParseExpr(module);

	if (module->lexer.curTok == TUT_TOK_ELSE)
	{
		Tut_GetToken(&module->lexer);
		exp->ifx.alt = ParseExpr(module);
	}
	else
		exp->ifx.alt = NULL;

	return exp;
}

static TutExpr* ParseWhile(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_WHILE, &module->lexer.context);
	Tut_GetToken(&module->lexer);

	exp->whilex.cond = ParseExpr(module);
	exp->whilex.body = ParseExpr(module);

	return exp;
}

static TutExpr* ParseStruct(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_STRUCT_DEF, &module->lexer.context);
	Tut_GetToken(&module->lexer);

	ExpectToken(module, TUT_TOK_IDENT);
	
	exp->structx.typetag = Tut_DefineType(&module->symbolTable, module->lexer.lexeme);
	Tut_GetToken(&module->lexer);

	EatToken(module, TUT_TOK_OPENCURLY);
	
	while (module->lexer.curTok != TUT_TOK_CLOSECURLY)
	{
		ExpectToken(module, TUT_TOK_IDENT);

		TutTypetagMember mem;
		mem.name = Tut_Strdup(module->lexer.lexeme);

		Tut_GetToken(&module->lexer);

		EatToken(module, TUT_TOK_COLON);
		
		mem.typetag = ParseType(module);

		if (module->lexer.curTok == TUT_TOK_SEMICOLON)
			Tut_GetToken(&module->lexer);

		Tut_ArrayPush(&exp->structx.typetag->user.members, &mem);
	}
	Tut_GetToken(&module->lexer);

	return exp;
}

static TutExpr* ParseCall(TutModule* module, TutExpr* pre)
{
	// TODO: Eventually allow for any expression to be callable
	if (pre->type != TUT_EXPR_IDENT)
		ParseError(module, "Expected identifier before '(' in call expression.\n");

	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_CALL, &module->lexer.context);
	
	Tut_GetToken(&module->lexer);

	exp->callx.func = pre;
	Tut_InitList(&exp->callx.args);
	
	while(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
	{
		TutExpr* arg = ParseExpr(module);
		Tut_ListAppend(&exp->callx.args, arg);
		
		if(module->lexer.curTok == TUT_TOK_COMMA)
			Tut_GetToken(&module->lexer);
		else if(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
			ParseError(module, "Expected ')' or ',' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	}
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseFactor(TutModule* module)
{
	switch(module->lexer.curTok)
	{
		case TUT_TOK_INT: return ParseInt(module);
		case TUT_TOK_FLOAT: return ParseFloat(module);

		case TUT_TOK_STRING: return ParseString(module);
		
		case TUT_TOK_VAR: return ParseVar(module);
		
		case TUT_TOK_IDENT: return ParseIdent(module);

		case TUT_TOK_IF: return ParseIf(module);

		case TUT_TOK_WHILE: return ParseWhile(module);

		case TUT_TOK_STRUCT: return ParseStruct(module);

		case TUT_TOK_FUNC: return ParseFunc(module);
		
		case TUT_TOK_EXTERN: return ParseExtern(module);

		case TUT_TOK_RETURN: return ParseReturn(module);
		
		case TUT_TOK_OPENPAREN: return ParseParen(module);
		
		case TUT_TOK_OPENCURLY: return ParseBlock(module);
	
		default:		
			ParseError(module, "Unexpected token '%s'\n", Tut_TokenRepr(module->lexer.curTok));			
			break;
	}

	return NULL;
}

static TutExpr* ParsePost(TutModule* module, TutExpr* pre)
{
	switch(module->lexer.curTok)
	{
		case TUT_TOK_OPENPAREN: return ParseCall(module, pre);
		
		default:
			return pre;
	}
}

static TutExpr* ParseUnary(TutModule* module)
{
	return ParsePost(module, ParseFactor(module));
}

static int GetTokenPrec(TutToken op)
{
	switch(op)
	{
		case TUT_TOK_ASSIGN: return 0;
		case TUT_TOK_LAND: case TUT_TOK_LOR: return 1;
		case TUT_TOK_LT: case TUT_TOK_GT: case TUT_TOK_LTE: case TUT_TOK_GTE: case TUT_TOK_EQUALS: case TUT_TOK_NEQUALS: return 2;
		case TUT_TOK_PLUS: case TUT_TOK_MINUS: return 3;
		case TUT_TOK_MUL: case TUT_TOK_DIV: return 4;
	
		default:
			return -1;
	}
}

static TutExpr* ParseBinRhs(TutModule* module, TutExpr* lhs, int eprec)
{
	while(TUT_TRUE)
	{
		int prec = GetTokenPrec(module->lexer.curTok);
		
		if(prec < eprec)
			return lhs;
		
		int op = module->lexer.curTok; 
		Tut_GetToken(&module->lexer);
		
		TutExpr* rhs = ParseUnary(module);
		if(GetTokenPrec(module->lexer.curTok) > prec)
			rhs = ParseBinRhs(module, rhs, prec + 1);
		
		TutExpr* exp = Tut_CreateExpr(TUT_EXPR_BIN, &module->lexer.context);
		
		exp->binx.lhs = lhs;
		exp->binx.rhs = rhs;
		exp->binx.op = op;
		
		lhs = exp;
	}
}

static TutExpr* ParseExpr(TutModule* module)
{
	return ParseBinRhs(module, ParseUnary(module), 0);
}

void Tut_ParseModule(TutModule* module)
{
	Tut_GetToken(&module->lexer);
	while(module->lexer.curTok != TUT_TOK_EOF)
	{
		TutExpr* exp = ParseExpr(module);
		Tut_ListAppend(&module->exprList, exp);
		
		if(module->lexer.curTok == TUT_TOK_SEMICOLON)
			Tut_GetToken(&module->lexer);
	}
}
