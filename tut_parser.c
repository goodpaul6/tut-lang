#include <stdarg.h>
#include <stdlib.h>

#include "tut_parser.h"
#include "tut_expr.h"

static void ParseError(TutModule* module, const char* format, ...)
{
	fprintf(stderr, "Parse Error (%s, %i): ", module->name, module->lexer.context.line);
	
	va_list args;
	va_start(args, format);
	
	vfprintf(stderr, format, args);
	
	va_end(args);
	
	exit(1);
}

static TutExpr* ParseExpr(TutModule* module);

static TutTypetag* ParseType(TutModule* module)
{
	// Attempt to create a primitive type tag (i.e bool, int, cstr, etc)
	TutTypetag* tag = Tut_CreatePrimitiveTypetag(module->lexer.lexeme);
	if(!tag)
		tag = Tut_RegisterType(&module->symbolTable, module->lexer.lexeme);
	
	Tut_GetToken(&module->lexer);
	return tag;
}

static TutExpr* ParseNumber(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_NUM, &module->lexer.context);
	exp->number = module->lexer.number;
	
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseString(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_NUM, &module->lexer.context);
	exp->string = Tut_Strdup(module->lexer.lexeme);

	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseVar(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_VAR, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	if(module->lexer.curTok != TUT_TOK_IDENT)
		ParseError(module, "Expected identifier but received '%s'\n", Tut_TokenRepr(module->lexer.curTok)); 
	
	exp->varx.name = Tut_Strdup(module->lexer.lexeme);
	Tut_GetToken(&module->lexer);
	
	if(module->lexer.curTok != TUT_TOK_COLON)
		ParseError(module, "Expected ':' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	
	Tut_GetToken(&module->lexer);
	
	exp->varx.varDecl = Tut_DeclareVariable(&module->symbolTable, exp->varx.name, ParseType(module));
	
	return exp;
}

static TutExpr* ParseIdent(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_VAR, &module->lexer.context);
				
	exp->varx.name = Tut_Strdup(module->lexer.lexeme);
	exp->varx.varDecl = Tut_GetVarDecl(&module->symbolTable, module->lexer.lexeme, -1);
	
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseFunc(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_FUNC, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	if(module->lexer.curTok != TUT_TOK_IDENT)
		ParseError(module, "Expected identifier but received '%s'\n", Tut_TokenRepr(module->lexer.curTok)); 
	
	exp->funcx.decl = Tut_DeclareVariable(&module->symbolTable, module->lexer.lexeme);
	Tut_GetToken(&module->lexer);
	
	Tut_PushCurFuncDecl(&module->symbolTable, exp->funcx.decl);
	
	if(module->lexer.curTok != TUT_TOK_OPENPAREN)
		ParseError(module, "Expected '(' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));	
	
	Tut_GetToken(&module->lexer);
	
	while(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
	{
		if(module->lexer.curTok != TUT_TOK_IDENT)
			ParseError(module, "Expected identifier but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
		
		char* name = Tut_Strdup(module->lexer.lexeme);
		Tut_GetToken(&module->lexer);
		
		if(module->lexer.curTok != TUT_TOK_COLON)
			ParseError(module, "Expected ':' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
		Tut_GetToken(&module->lexer);
		
		TutTypetag* tag = ParseType(module);
		
		Tut_DeclareArgument(&module->symbolTable, name, tag);
		
		if(module->lexer.curTok == TUT_TOK_COMMA)
			Tut_GetToken(&module->lexer);
		else if(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
			ParseError(module, "Expected ')' or ',' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	}
	Tut_GetToken(&module->lexer);	
	
	if(module->lexer.curTok != TUT_TOK_COLON)
		ParseError(module, "Expected ':' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	Tut_GetToken(&module->lexer);
	
	exp->funcx.decl->returnType = ParseType(module);
	exp->funcx.body = ParseExpr(module);
	
	return exp;
}

static TutExpr* ParseReturn(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_RETURN, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
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
		ParseError("Expected matching ')' after previous '(' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	Tut_GetToken(&module->lexer);
	
	return exp;	
}

static TutExpr* ParseBlock(TutModule* module)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_BLOCK, &module->lexer.context);
	Tut_GetToken(&module->lexer);
	
	while(module->lexer.curTok != TUT_TOK_CLOSECURLY)
	{
		TutExpr* exp = ParseExpr(module);
		if(module->lexer.curTok == TUT_TOK_SEMICOLON)
			Tut_GetToken(&module->lexer);
		
		Tut_ListAppend(&exp->blockList, exp);
	}
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseCall(TutModule* module, TutExpr* pre)
{
	TutExpr* exp = Tut_CreateExpr(TUT_EXPR_CALL, &module->lexer.context);
	
	exp->callx.func = pre;
	Tut_InitList(&exp->callx.args);
	
	while(module->lexer.curTok != TUT_TOK_CLOSEPAREN)
	{
		TutExpr* arg = ParseExpr(module);
		Tut_ListAppend(&exp->callx.args, arg);
		
		if(module->lexer.curTok == TUT_TOK_COMMA)
			Tut_GetToken(&module->lexer);
		else if(module->lexer.curTok != TUT_TOK_CLOSECURLY)
			ParseError("Expected ')' or ',' but received '%s'\n", Tut_TokenRepr(module->lexer.curTok));
	}
	Tut_GetToken(&module->lexer);
	
	return exp;
}

static TutExpr* ParseFactor(TutModule* module)
{
	switch(module->lexer.curTok)
	{
		case TUT_TOK_NUMBER: return ParseNumber(module);
		case TUT_TOK_STRING: return ParseString(module);
		
		case TUT_TOK_VAR: return ParseVar(module);
		
		case TUT_TOK_IDENT: return ParseIdent(module);
		
		case TUT_TOK_FUNC: return ParseFunc(module);
		
		case TUT_TOK_RETURN: return ParseReturn(module);
		
		case TUT_TOK_OPENPAREN: return ParseParen(module);
		
		case TUT_TOK_OPENCURLY: return ParseBlock(module);
	
		default:		
			ParseError(module, "Unexpected token '%s'\n", Tut_TokenRepr(module->lexer.curTok));			
			break;
	}
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

static int GetTokenPrec(TutToken token)
{
	switch(op)
	{
		case TUT_TOK_ASSIGN: return 0;
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
	while(module->lexer.curTok != TUT_TOK_EOF)
	{
		TutExpr* exp = ParseExpr(module);
		Tut_ListAppend(&module->exprList, exp);
		
		if(module->lexer.curTok == TUT_TOK_SEMICOLON)
			Tut_GetToken(&module->lexer);
	}
}
