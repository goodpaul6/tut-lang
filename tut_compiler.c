#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#include "tut_compiler.h"
#include "tut_codegen.h"
#include "tut_opcodes.h"
#include "tut_expr.h"

static const char* Flags[TUT_CFLAG_COUNT] =
{
	NULL,
	NULL
};

static void CompilerError(TutExpr* exp, const char* format, ...)
{
	char* lineEnd = strchr(exp->context.lineStart, '\n');
	if (lineEnd)
		fprintf(stderr, "%.*s\n", (int)(lineEnd - exp->context.lineStart), exp->context.lineStart);
	else
		fprintf(stderr, "%s\n", exp->context.lineStart);
	fprintf(stderr, "Error (%s, %i): ", exp->context.filename, exp->context.line);

	va_list args;
	
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	if (exp->context.filename)
	{
		static char buf[1024];

		if (Flags[TUT_CFLAG_OPEN_ERROR_GEANY_PATH])
		{
			sprintf(buf, "%s +%d --column %d %s", Flags[TUT_CFLAG_OPEN_ERROR_GEANY_PATH], exp->context.line, (int)(exp->context.current - exp->context.lineStart), exp->context.filename);
			system(buf);
		}
		else if (Flags[TUT_CFLAG_OPEN_ERROR_NPP_PATH])
		{
			sprintf(buf, "%s -n%d -c%d %s", Flags[TUT_CFLAG_OPEN_ERROR_NPP_PATH], exp->context.line, (int)(exp->context.current - exp->context.lineStart), exp->context.filename);
			system(buf);
		}
	}

	exit(1);
}

static TutTypetagMember* GetMember(TutTypetag* tag, const char* memberName)
{
	assert(tag);
	assert(tag->type == TUT_TYPETAG_USERTYPE);

	for (int i = 0; i < tag->user.members.length; ++i)
	{
		TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);
		if (strcmp(mem->name, memberName) == 0)
			return mem;
	}

	return NULL;
}

typedef struct
{
	TutExpr* root;
	TutVarDecl* decl;
	int offset;
} Lvalue;

static TutBool GetLvalue(Lvalue* value, TutExpr* exp, const char* memberName)
{
	assert(exp);
	assert(exp->typetag);

	if (memberName)
		assert(exp->typetag->type == TUT_TYPETAG_USERTYPE);

	switch (exp->type)
	{
		case TUT_EXPR_VAR:
		case TUT_EXPR_IDENT:
		{
			value->root = exp;
			value->decl = exp->varx.decl;

			if (memberName)
				value->offset += GetMember(exp->typetag, memberName)->offset;

			return TUT_TRUE;
		} break;

		case TUT_EXPR_DOT:
		{
			if (GetLvalue(value, exp->dotx.value, exp->dotx.memberName))
			{
				if (memberName)
					value->offset += GetMember(exp->typetag, memberName)->offset;

				return TUT_TRUE;
			}

			return TUT_FALSE;
		} break;

		case TUT_EXPR_ARROW:
		{
			TutTypetag* tag = exp->dotx.value->typetag;

			assert(tag->type == TUT_TYPETAG_REF);
			assert(tag->ref.value);
			assert(tag->ref.value->type == TUT_TYPETAG_USERTYPE);

			value->root = exp->dotx.value;
			value->decl = NULL;
			value->offset += GetMember(tag->ref.value, exp->dotx.memberName)->offset;

			return TUT_TRUE;
		} break;

		case TUT_EXPR_PAREN:
		{
			return GetLvalue(value, exp->parenExpr, memberName);
		} break;

		default:
			return TUT_FALSE;
	}
}

static void FinalizeTypes(TutModule* module)
{
	TUT_LIST_EACH(node, module->symbolTable->usertypes)
	{
		TutTypetag* t = node->value;
		if (t->type == TUT_TYPETAG_USERTYPE)
		{
			if (!t->user.defined)
				Tut_ErrorExit("Attempted to use undefined type '%s'.\n", t->user.name);

			if (t->user.members.length == 0)
				Tut_ErrorExit("Type '%s' has no members.\n", t->user.name);

			int totalSize = 0;
			
			for(size_t i = 0; i < t->user.members.length; ++i)
			{
				TutTypetagMember* mem = Tut_ArrayGet(&t->user.members, i);

				mem->offset = totalSize;
				totalSize += Tut_GetTypetagSize(mem->typetag);
			}
		}
	}
}

static void ResolveVariableIndices(TutModule* module)
{
	int globalIndex = 0;

	TUT_LIST_EACH(node, module->symbolTable->globals)
	{
		TutVarDecl* decl = node->value;
		decl->index = globalIndex;

		globalIndex += Tut_GetTypetagSize(decl->typetag);
	}

	TUT_LIST_EACH(node, module->symbolTable->functions)
	{
		TutFuncDecl* decl = node->value;
		if (decl->type == TUT_FUNC_DECL_NORMAL)
		{
			// Calculate the total size of the argument batch
			int totalArgSize = 0;

			TUT_LIST_EACH(argNode, decl->args)
			{
				TutVarDecl* decl = argNode->value;

				totalArgSize += Tut_GetTypetagSize(decl->typetag);
			}

			// The index of each argument is set such that
			// the first argument has the most negative index

			TUT_LIST_EACH(argNode, decl->args)
			{
				TutVarDecl* decl = argNode->value;
				
				decl->index = -totalArgSize;
				totalArgSize -= Tut_GetTypetagSize(decl->typetag);
			}

			int varIndex = 0;
			TUT_LIST_EACH(varNode, decl->locals)
			{
				TutVarDecl* decl = varNode->value;

				decl->index = varIndex;
				varIndex += Tut_GetTypetagSize(decl->typetag);
			}
		}
	}
}

static void ResolveSymbols(TutModule* module, TutExpr* exp)
{
	assert(exp);

	switch (exp->type)
	{
		case TUT_EXPR_TRUE:
		case TUT_EXPR_FALSE:
		case TUT_EXPR_INT:
		case TUT_EXPR_FLOAT:
		case TUT_EXPR_STR:
		case TUT_EXPR_STRUCT_DEF:
		{
		} break;

		case TUT_EXPR_SIZEOF:
		{
			ResolveSymbols(module, exp->sizeofx.value);
		} break;

		case TUT_EXPR_CAST:
		{
			ResolveSymbols(module, exp->castx.value);
		} break;

		case TUT_EXPR_VAR:
		{
			// Already resolved right?
			assert(exp->varx.decl);
		} break;

		case TUT_EXPR_IDENT:
		{
			if (!exp->varx.decl)
			{
				exp->varx.decl = Tut_GetVarDecl(module->symbolTable, exp->varx.name, 0);
				if (!exp->varx.decl)
				{
					TutFuncDecl* decl = Tut_GetFuncDecl(module->symbolTable, exp->varx.name);
					if (!decl)
					{
						TutTypetag* tag = Tut_CreatePrimitiveTypetag(exp->varx.name);
						if (!tag)
						{
							tag = Tut_GetType(module->symbolTable, exp->varx.name);
							if (!tag)
								CompilerError(exp, "Attempted to access undeclared variable '%s'.\n", exp->varx.name);
							exp->varx.typetag = tag;
						}
						else
							exp->varx.typetag = tag;
					}
					else
						exp->varx.funcDecl = decl;
				}
			}
		} break;

		case TUT_EXPR_BIN:
		{
			ResolveSymbols(module, exp->binx.lhs);
			ResolveSymbols(module, exp->binx.rhs);
		} break;

		case TUT_EXPR_UNARY:
		{
			ResolveSymbols(module, exp->unaryx.value);
		} break;

		case TUT_EXPR_PAREN:
		{
			ResolveSymbols(module, exp->parenExpr);
		} break;

		case TUT_EXPR_ARROW:
		case TUT_EXPR_DOT:
		{
			ResolveSymbols(module, exp->dotx.value);
		} break;
		 
		case TUT_EXPR_CALL:
		{
			ResolveSymbols(module, exp->callx.func);
			TUT_LIST_EACH(node, exp->callx.args)
				ResolveSymbols(module, node->value);
		} break;

		case TUT_EXPR_BLOCK:
		{
			TUT_LIST_EACH(node, exp->blockList)
				ResolveSymbols(module, node->value);
		} break;

		case TUT_EXPR_IF:
		{
			ResolveSymbols(module, exp->ifx.cond);
			ResolveSymbols(module, exp->ifx.body);

			if (exp->ifx.alt)
				ResolveSymbols(module, exp->ifx.alt);
		} break;

		case TUT_EXPR_WHILE:
		{
			ResolveSymbols(module, exp->whilex.cond);
			ResolveSymbols(module, exp->whilex.body);
		} break;

		case TUT_EXPR_RETURN:
		{
			if (exp->retx.value)
				ResolveSymbols(module, exp->retx.value);
		} break;

		case TUT_EXPR_FUNC:
		{
			ResolveSymbols(module, exp->funcx.body);
		} break;
	}
}

static void ResolveTypes(TutModule* module, TutExpr* exp)
{
	assert(exp);

	switch (exp->type)
	{
		case TUT_EXPR_SIZEOF:
		{
			ResolveTypes(module, exp->sizeofx.value);
			exp->typetag = Tut_CreatePrimitiveTypetag("int");
		} break;
		
		case TUT_EXPR_NULL:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("ref");
		} break;

		case TUT_EXPR_TRUE:
		case TUT_EXPR_FALSE:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("bool");
		} break;

		case TUT_EXPR_INT:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("int");
		} break;

		case TUT_EXPR_FLOAT:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("float");
		} break;

		case TUT_EXPR_STR:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("cstr");
		} break;

		case TUT_EXPR_VAR:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
		} break;

		case TUT_EXPR_IDENT:
		{
			assert(exp->varx.decl || exp->varx.funcDecl || exp->varx.typetag);

			if (exp->varx.decl)
				exp->typetag = exp->varx.decl->typetag;
			else if (exp->varx.funcDecl)
				exp->typetag = exp->varx.funcDecl->typetag;
			else if (exp->varx.typetag)
				exp->typetag = exp->varx.typetag;
		} break;

		case TUT_EXPR_UNARY:
		{
			ResolveTypes(module, exp->unaryx.value);
			
			assert(exp->unaryx.op == TUT_TOK_MINUS ||
				   exp->unaryx.op == TUT_TOK_MUL ||
				   exp->unaryx.op == TUT_TOK_AND);

			if (exp->unaryx.op == TUT_TOK_MINUS)
			{
				if (exp->unaryx.value->typetag->type != TUT_TYPETAG_INT && exp->unaryx.value->typetag->type != TUT_TYPETAG_FLOAT)
					CompilerError(exp, "Unary operator '%s' cannot be applied to value of type '%s'.\n", Tut_TokenRepr(exp->unaryx.op), Tut_TypetagRepr(exp->unaryx.value->typetag));

				exp->typetag = exp->unaryx.value->typetag;
			}
			else if (exp->unaryx.op == TUT_TOK_MUL)
			{
				if (exp->unaryx.value->typetag->type != TUT_TYPETAG_REF)
					CompilerError(exp, "Cannot dereference value of type '%s'.\n", Tut_TypetagRepr(exp->unaryx.value->typetag));
				if (!exp->unaryx.value->typetag->ref.value)
					CompilerError(exp, "Cannot dereference unspecified reference value.\n");
				
				exp->typetag = exp->unaryx.value->typetag->ref.value;
			}
			else if (exp->unaryx.op == TUT_TOK_AND)
			{
				exp->typetag = Tut_CreatePrimitiveTypetag("ref");
				assert(exp->typetag);

				exp->typetag->ref.value = exp->unaryx.value->typetag;
			}
		} break;

		case TUT_EXPR_BIN:
		{
			ResolveTypes(module, exp->binx.lhs);
			ResolveTypes(module, exp->binx.rhs);
		
			assert(exp->binx.lhs->typetag && exp->binx.rhs->typetag);
			
			if (exp->binx.op != TUT_TOK_ASSIGN)
			{
				if (exp->binx.lhs->typetag->type == TUT_TYPETAG_VOID || exp->binx.lhs->typetag->type == TUT_TYPETAG_USERTYPE)
					CompilerError(exp, "Cannot apply binary operation '%s' to type '%s'.\n", Tut_TokenRepr(exp->binx.op), Tut_TypetagRepr(exp->binx.lhs->typetag));

				if (!Tut_CompareTypes(exp->binx.lhs->typetag, exp->binx.rhs->typetag))
					CompilerError(exp, "Mismatched types in binary operation (%s, %s).\n", Tut_TypetagRepr(exp->binx.lhs->typetag), Tut_TypetagRepr(exp->binx.rhs->typetag));
				
				if (exp->binx.op != TUT_TOK_EQUALS && exp->binx.op != TUT_TOK_NEQUALS &&
					exp->binx.op != TUT_TOK_LT && exp->binx.op != TUT_TOK_GT &&
					exp->binx.op != TUT_TOK_LTE && exp->binx.op != TUT_TOK_GTE &&
					exp->binx.op != TUT_TOK_LAND && exp->binx.op != TUT_TOK_LOR)
					exp->typetag = exp->binx.lhs->typetag;
				else
					exp->typetag = Tut_CreatePrimitiveTypetag("bool");
			}
			else
			{
				if (exp->binx.lhs->type != TUT_EXPR_VAR)
				{
					// IMPORTANT: Autoconversion from 'str' to 'cstr'
					if(!Tut_CanAssignTypes(exp->binx.rhs->typetag, exp->binx.lhs->typetag))
						CompilerError(exp, "Mismatched types in assignment operation (%s, %s).\n", Tut_TypetagRepr(exp->binx.lhs->typetag), Tut_TypetagRepr(exp->binx.rhs->typetag));
				}
				else
				{
					assert(exp->binx.lhs->varx.decl);

					if (!Tut_CanAssignTypes(exp->binx.rhs->typetag, exp->binx.lhs->varx.decl->typetag))
						CompilerError(exp, "Mismatched types in assignment operation (%s, %s).\n", Tut_TypetagRepr(exp->binx.lhs->varx.decl->typetag), Tut_TypetagRepr(exp->binx.rhs->typetag));
				}
			}
		} break;

		case TUT_EXPR_PAREN:
		{
			ResolveTypes(module, exp->parenExpr);
			exp->typetag = exp->parenExpr->typetag;
		} break;
		
		case TUT_EXPR_ARROW:
		case TUT_EXPR_DOT:
		{
			ResolveTypes(module, exp->dotx.value);
			assert(exp->dotx.value->typetag);

			TutTypetag* tag = NULL;

			if (exp->type == TUT_EXPR_DOT)
			{
				if (exp->dotx.value->typetag->type != TUT_TYPETAG_USERTYPE)
					CompilerError(exp, "Type '%s' has no members.\n", Tut_TypetagRepr(exp->dotx.value->typetag));
				
				tag = exp->dotx.value->typetag;
			}
			else if (exp->type == TUT_EXPR_ARROW)
			{
				if (exp->dotx.value->typetag->type != TUT_TYPETAG_REF)
					CompilerError(exp, "Type '%s' is not a reference type so you cannot use '->' to index it.\n", Tut_TypetagRepr(exp->dotx.value->typetag));
				if (!exp->dotx.value->typetag->ref.value)
					CompilerError(exp, "Cannot use '->' operator on unspecified reference.\n");
				if (exp->dotx.value->typetag->ref.value->type != TUT_TYPETAG_USERTYPE)
					CompilerError(exp, "Type '%s' has no members.\n", Tut_TypetagRepr(exp->dotx.value->typetag->ref.value));
			
				tag = exp->dotx.value->typetag->ref.value;
			}

			assert(tag);
			TutTypetagMember* found = GetMember(tag, exp->dotx.memberName);

			if (!found)
				CompilerError(exp, "Type '%s' has no member named '%s'.\n", Tut_TypetagRepr(tag), exp->dotx.memberName);
			
			exp->typetag = found->typetag;
		} break;

		case TUT_EXPR_CALL:
		{
			ResolveTypes(module, exp->callx.func);

			if (exp->callx.func->typetag->type != TUT_TYPETAG_FUNC)
				CompilerError(exp, "Attempted to call non-function value of type '%s'.\n", Tut_TypetagRepr(exp->callx.func->typetag));

			assert(exp->callx.func->typetag->func.ret);

			exp->typetag = exp->callx.func->typetag->func.ret;

			if (!exp->callx.func->typetag->func.hasVarargs)
			{
				if (exp->callx.args.length != exp->callx.func->typetag->func.args.length)
					CompilerError(exp, "Invalid number of arguments in function call; expected %d arguments but passed %d\n",
						exp->callx.args.length, exp->callx.func->typetag->func.args.length);
			}
			else if (exp->callx.args.length < exp->callx.func->typetag->func.args.length)
				CompilerError(exp, "Expected at least %d arguments in function call but passed %d\n",
					exp->callx.func->typetag->func.args.length, exp->callx.args.length);

			TUT_LIST_EACH(node, exp->callx.args)
				ResolveTypes(module, node->value);

			TutListNode* argNode = exp->callx.args.head;
			TutListNode* tagNode = exp->callx.func->typetag->func.args.head;

			int i = 0;
			while (argNode && tagNode)
			{
				TutExpr* arg = argNode->value;
				TutTypetag* tag = tagNode->value;

				if (!Tut_CanAssignTypes(arg->typetag, tag))
					CompilerError(arg, "Argument %d was supposed to be a '%s' but you passed a '%s'\n",
							i + 1, Tut_TypetagRepr(tag), Tut_TypetagRepr(arg->typetag));

				argNode = argNode->next;
				tagNode = tagNode->next;
				++i;
			}
		} break;

		case TUT_EXPR_CAST:
		{
			ResolveTypes(module, exp->castx.value);
			exp->typetag = exp->castx.typetag;
		} break;

		case TUT_EXPR_BLOCK:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");

			TUT_LIST_EACH(node, exp->blockList)
				ResolveTypes(module, node->value);
		} break;

		case TUT_EXPR_IF:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");

			ResolveTypes(module, exp->ifx.cond);
			ResolveTypes(module, exp->ifx.body);

			if (exp->ifx.alt)
				ResolveTypes(module, exp->ifx.alt);
		} break;

		case TUT_EXPR_WHILE:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
			
			ResolveTypes(module, exp->whilex.cond);
			ResolveTypes(module, exp->whilex.body);
		} break;

		case TUT_EXPR_STRUCT_DEF:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
		} break;

		case TUT_EXPR_FUNC:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
			
			ResolveTypes(module, exp->funcx.body);
		} break;
		
		case TUT_EXPR_RETURN:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");

			assert(exp->retx.parent);
			assert(exp->retx.parent->typetag->func.ret);

			if (exp->retx.value)
			{
				ResolveTypes(module, exp->retx.value);

				if (!Tut_CompareTypes(exp->retx.parent->typetag->func.ret, exp->retx.value->typetag))
					CompilerError(exp, "Type of value in return statement '%s' does not match return type of enclosing function '%s'\n", 
						Tut_TypetagRepr(exp->retx.value->typetag), Tut_TypetagRepr(exp->retx.parent->typetag->func.ret));
			}
			else if (exp->retx.parent->typetag->func.ret->type != TUT_TYPETAG_VOID)
				CompilerError(exp, "Function must return a value.\n");
		} break;
	}
}

static void CompileValue(TutModule* module, TutVM* vm, TutExpr* exp);

static void CompileAssign(TutModule* module, TutVM* vm, TutExpr* lhs)
{
	assert(lhs);
	assert(lhs->typetag);

	switch (lhs->type)
	{
		case TUT_EXPR_VAR:
		case TUT_EXPR_IDENT:
		{
			assert(lhs->varx.decl);
			Tut_EmitSet(vm, !lhs->varx.decl->parent, lhs->varx.decl->index, Tut_GetTypetagSize(lhs->varx.decl->typetag));
		} break;

		case TUT_EXPR_DOT:
		{
			Lvalue value = { 0 };

			// p.scene.x
			if (!GetLvalue(&value, lhs->dotx.value, lhs->dotx.memberName))
				CompilerError(lhs, "Invalid lhs in assignment expression.\n");

			Tut_EmitSet(vm, !value.decl->parent, value.decl->index + value.offset, Tut_GetTypetagSize(lhs->typetag));
		} break;

		case TUT_EXPR_UNARY:
		{
			if (lhs->unaryx.op != TUT_TOK_MUL)
				CompilerError(lhs, "Invalid lhs in assignment expression.\n");
			
			assert(lhs->unaryx.value->typetag);
			assert(lhs->unaryx.value->typetag->type == TUT_TYPETAG_REF);
			assert(lhs->unaryx.value->typetag->ref.value);

			// Push ref onto stack
			CompileValue(module, vm, lhs->unaryx.value);

			// Memcpy &stack[currentPos - size] into ref 
			Tut_EmitSetRef(vm, Tut_GetTypetagSize(lhs->unaryx.value->typetag->ref.value), 0);
		} break;

		case TUT_EXPR_ARROW:
		{
			assert(lhs->dotx.value->typetag);
			assert(lhs->dotx.value->typetag->type == TUT_TYPETAG_REF);
			assert(lhs->dotx.value->typetag->ref.value->type == TUT_TYPETAG_USERTYPE);
		
			// Push ref onto stack
			CompileValue(module, vm, lhs->dotx.value);
			
			TutTypetagMember* mem = GetMember(lhs->dotx.value->typetag->ref.value, lhs->dotx.memberName);
			
			assert(mem);
			assert(mem->offset >= 0);

			// Pop (size) values off the stack and put them into &ref[mem->offset]
			Tut_EmitSetRef(vm, Tut_GetTypetagSize(mem->typetag), mem->offset);
		} break;

		default:
			CompilerError(lhs, "Invalid lhs in assignment statement.\n");
			break;
	}
}

static void CompileCall(TutModule* module, TutVM* vm, TutExpr* exp, TutBool discardReturnValue)
{
	assert(exp->callx.func->typetag);
	assert(exp->callx.func->typetag->type == TUT_TYPETAG_FUNC);

	int totalCount = 0;

	TUT_LIST_EACH(node, exp->callx.args)
	{
		TutExpr* arg = node->value;

		assert(arg->typetag);
		totalCount += Tut_GetTypetagSize(arg->typetag);

		CompileValue(module, vm, node->value);
	}
	
	CompileValue(module, vm, exp->callx.func);
	Tut_EmitCall(vm, totalCount);

	if (discardReturnValue && exp->callx.func->typetag->func.ret->type != TUT_TYPETAG_VOID)
	{
		TutTypetag* ret = exp->callx.func->typetag->func.ret;
		Tut_EmitPop(vm, Tut_GetTypetagSize(ret));
	}
}

static void CompileValue(TutModule* module, TutVM* vm, TutExpr* exp)
{
	assert(exp);
	assert(exp->typetag);

	switch (exp->type)
	{
		case TUT_EXPR_SIZEOF:
		{
			assert(exp->sizeofx.value->typetag);
			Tut_EmitPushInt(vm, Tut_GetTypetagSize(exp->sizeofx.value->typetag) * sizeof(TutObject));
		} break;

		case TUT_EXPR_TRUE:
		{
			Tut_EmitOp(vm, TUT_OP_PUSH_TRUE);
		} break;

		case TUT_EXPR_FALSE:
		{
			Tut_EmitOp(vm, TUT_OP_PUSH_FALSE);
		} break;

		case TUT_EXPR_NULL:
		{
			Tut_EmitOp(vm, TUT_OP_PUSH_NULL);
		} break;

		case TUT_EXPR_INT:
		{
			Tut_EmitPushInt(vm, exp->intVal);
		} break;

		case TUT_EXPR_FLOAT:
		{
			Tut_EmitPushFloat(vm, exp->floatVal);
		} break;

		case TUT_EXPR_STR:
		{
			Tut_EmitPushStr(vm, exp->string);
		} break;
		
		case TUT_EXPR_IDENT:
		{
			assert(exp->typetag);

			if (exp->varx.typetag)
				CompilerError(exp, "Cannot use type '%s' as value.\n", Tut_TypetagRepr(exp->varx.typetag));

			assert((exp->varx.decl && exp->varx.decl->index != TUT_VAR_DECL_INDEX_UNDEFINED) || 
				exp->varx.funcDecl);
			
			if (exp->varx.decl)
			{
				// BAM! Copy the entire thing at once
				Tut_EmitGet(vm, !exp->varx.decl->parent, exp->varx.decl->index, Tut_GetTypetagSize(exp->varx.decl->typetag));
			}
			else if (exp->varx.funcDecl)
				Tut_EmitMakeFunc(vm, exp->varx.funcDecl->type == TUT_FUNC_DECL_EXTERN, exp->varx.funcDecl->index);
		} break;

		case TUT_EXPR_UNARY:
		{
			assert(exp->unaryx.op == TUT_TOK_MINUS ||
				   exp->unaryx.op == TUT_TOK_AND ||
				   exp->unaryx.op == TUT_TOK_MUL);

			if (exp->unaryx.op == TUT_TOK_MINUS)
			{
				assert(exp->typetag->type == TUT_TYPETAG_INT || exp->typetag->type == TUT_TYPETAG_FLOAT);

				CompileValue(module, vm, exp->unaryx.value);

				if (exp->typetag->type == TUT_TYPETAG_INT)
					Tut_EmitOp(vm, TUT_OP_INEG);
				else if (exp->typetag->type == TUT_TYPETAG_FLOAT)
					Tut_EmitOp(vm, TUT_OP_FNEG);
			}
			else if (exp->unaryx.op == TUT_TOK_AND)
			{
				Lvalue value = { 0 };

				if (!GetLvalue(&value, exp->unaryx.value, NULL))
					CompilerError(exp, "Cannot create a reference to this value (possibly a temporary value).\n");

				if(value.decl)
					Tut_EmitMakeVarRef(vm, !value.decl->parent, value.decl->index + value.offset);
				else
				{
					// The 'root' value is the reference
					assert(value.root);
					CompileValue(module, vm, value.root);

					Tut_EmitMakeDynRef(vm, value.offset);
				}
			}
			else if (exp->unaryx.op == TUT_TOK_MUL)
			{
				// Push ref onto stack
				CompileValue(module, vm, exp->unaryx.value);
				// memcpy ref values onto stack
				Tut_EmitGetRef(vm, Tut_GetTypetagSize(exp->unaryx.value->typetag->ref.value), 0);
			}
		} break;

		case TUT_EXPR_BIN:
		{
			if (exp->binx.op != TUT_TOK_ASSIGN)
			{
				CompileValue(module, vm, exp->binx.lhs);
				CompileValue(module, vm, exp->binx.rhs);

				assert(exp->binx.lhs->typetag);
				if (exp->binx.lhs->typetag->type == TUT_TYPETAG_INT)
				{
					if (exp->binx.op == TUT_TOK_PLUS) Tut_EmitOp(vm, TUT_OP_ADDI);
					else if (exp->binx.op == TUT_TOK_MINUS) Tut_EmitOp(vm, TUT_OP_SUBI);
					else if (exp->binx.op == TUT_TOK_MUL) Tut_EmitOp(vm, TUT_OP_MULI);
					else if (exp->binx.op == TUT_TOK_DIV) Tut_EmitOp(vm, TUT_OP_DIVI);
					else if (exp->binx.op == TUT_TOK_LT) Tut_EmitOp(vm, TUT_OP_ILT);
					else if (exp->binx.op == TUT_TOK_GT) Tut_EmitOp(vm, TUT_OP_IGT);
					else if (exp->binx.op == TUT_TOK_LTE) Tut_EmitOp(vm, TUT_OP_ILTE);
					else if (exp->binx.op == TUT_TOK_GTE) Tut_EmitOp(vm, TUT_OP_IGTE);
					else if (exp->binx.op == TUT_TOK_EQUALS) Tut_EmitOp(vm, TUT_OP_IEQ);
					else if (exp->binx.op == TUT_TOK_NEQUALS)
					{
						Tut_EmitOp(vm, TUT_OP_IEQ);
						Tut_EmitOp(vm, TUT_OP_LNOT);
					}
					else CompilerError(exp, "Invalid binary operator '%s' for operation involving type '%s'.\n", Tut_TokenRepr(exp->binx.op), Tut_TypetagRepr(exp->binx.lhs->typetag));
				}
				else if (exp->binx.lhs->typetag->type == TUT_TYPETAG_FLOAT)
				{
					if (exp->binx.op == TUT_TOK_PLUS) Tut_EmitOp(vm, TUT_OP_ADDF);
					else if (exp->binx.op == TUT_TOK_MINUS) Tut_EmitOp(vm, TUT_OP_SUBF);
					else if (exp->binx.op == TUT_TOK_MUL) Tut_EmitOp(vm, TUT_OP_MULF);
					else if (exp->binx.op == TUT_TOK_DIV) Tut_EmitOp(vm, TUT_OP_DIVF);
					else if (exp->binx.op == TUT_TOK_LT) Tut_EmitOp(vm, TUT_OP_FLT);
					else if (exp->binx.op == TUT_TOK_GT) Tut_EmitOp(vm, TUT_OP_FGT);
					else if (exp->binx.op == TUT_TOK_LTE) Tut_EmitOp(vm, TUT_OP_FLTE);
					else if (exp->binx.op == TUT_TOK_GTE) Tut_EmitOp(vm, TUT_OP_FGTE);
					else if (exp->binx.op == TUT_TOK_EQUALS) Tut_EmitOp(vm, TUT_OP_FEQ);
					else if (exp->binx.op == TUT_TOK_NEQUALS)
					{
						Tut_EmitOp(vm, TUT_OP_FEQ);
						Tut_EmitOp(vm, TUT_OP_LNOT);
					}
					else CompilerError(exp, "Invalid binary operator '%s' for operation involving type '%s'.\n", Tut_TokenRepr(exp->binx.op), Tut_TypetagRepr(exp->binx.lhs->typetag));
				}
				else if (exp->binx.lhs->typetag->type == TUT_TYPETAG_STR)
				{
					if (exp->binx.op == TUT_TOK_EQUALS) Tut_EmitOp(vm, TUT_OP_SEQ);
					else if (exp->binx.op == TUT_TOK_NEQUALS)
					{
						Tut_EmitOp(vm, TUT_OP_SEQ);
						Tut_EmitOp(vm, TUT_OP_LNOT);
					}
					else CompilerError(exp, "Invalid binary operator '%s' for operation involving type '%s'.\n", Tut_TokenRepr(exp->binx.op), Tut_TypetagRepr(exp->binx.lhs->typetag));
				}
				else if (exp->binx.lhs->typetag->type == TUT_TYPETAG_BOOL)
				{
					if (exp->binx.op == TUT_TOK_LAND) Tut_EmitOp(vm, TUT_OP_LAND);
					else if (exp->binx.op == TUT_TOK_LOR) Tut_EmitOp(vm, TUT_OP_LOR);
					else if (exp->binx.op == TUT_TOK_EQUALS) Tut_EmitOp(vm, TUT_OP_BEQ);
					else if (exp->binx.op == TUT_TOK_NEQUALS)
					{
						Tut_EmitOp(vm, TUT_OP_BEQ);
						Tut_EmitOp(vm, TUT_OP_LNOT);
					}
					else CompilerError(exp, "Invalid binary operator '%s' for operation involving type '%s'.\n", Tut_TokenRepr(exp->binx.op), Tut_TypetagRepr(exp->binx.lhs->typetag));
				}
				else if (exp->binx.lhs->typetag->type == TUT_TYPETAG_REF)
				{
					if (exp->binx.op == TUT_TOK_EQUALS) Tut_EmitOp(vm, TUT_OP_REQ);
					else if (exp->binx.op == TUT_TOK_NEQUALS)
					{
						Tut_EmitOp(vm, TUT_OP_REQ);
						Tut_EmitOp(vm, TUT_OP_LNOT);
					}
					else CompilerError(exp, "Invalid binary operator '%s' for operation involving type '%s'.\n", Tut_TokenRepr(exp->binx.op), Tut_TypetagRepr(exp->binx.lhs->typetag));
				}
				else
					CompilerError(exp, "(INTERNAL) Invalid lhs type in binary operation. Invalid type resolution, possibly?\n");
			}
			else
				CompilerError(exp, "Assignment found when expecting a value.\n");
		} break;

		case TUT_EXPR_PAREN:
		{
			CompileValue(module, vm, exp->parenExpr);
		} break;

		case TUT_EXPR_DOT:
		{
			assert(exp->dotx.value->typetag);
			assert(exp->dotx.value->typetag->type == TUT_TYPETAG_USERTYPE);

			CompileValue(module, vm, exp->dotx.value);

			// Every value of the structure is pushed onto the stack
			//                x   y   z
			// stack = [10, { 20, 30, 40 }], where curly braces denote values in the struct
			// if we wanna get at y, we have to pop z and then move y's value to x
	
			TutTypetag* tag = exp->dotx.value->typetag;
			TutTypetagMember* mem = GetMember(tag, exp->dotx.memberName);

			int size = Tut_GetTypetagSize(tag);
			int memberSize = Tut_GetTypetagSize(exp->typetag);

			// in the case illustrated above, (size = 3, mem->offset = 1, and memberSize = 1)
			// so amountToPop = 1
			int amountToPop = size - mem->offset - memberSize;
			int amountToMove = mem->offset;

			Tut_EmitPop(vm, amountToPop);
			Tut_EmitMove(vm, memberSize, amountToMove);
		} break;

		case TUT_EXPR_ARROW:
		{
			assert(exp->dotx.value->typetag);
			assert(exp->dotx.value->typetag->type == TUT_TYPETAG_REF);
			assert(exp->dotx.value->typetag->ref.value->type == TUT_TYPETAG_USERTYPE);

			// Push ref onto stack
			CompileValue(module, vm, exp->dotx.value);

			TutTypetagMember* mem = GetMember(exp->dotx.value->typetag->ref.value, exp->dotx.memberName);
			assert(mem);

			Tut_EmitGetRef(vm, Tut_GetTypetagSize(mem->typetag), mem->offset);
		} break;
		
		case TUT_EXPR_CAST:
		{
			CompileValue(module, vm, exp->castx.value);
		} break;

		case TUT_EXPR_CALL:
		{
			CompileCall(module, vm, exp, TUT_FALSE);
		} break;

		default:
			CompilerError(exp, "Found statement when expection expression.\n");
			break;
	}
}

static void CompileStatement(TutModule* module, TutVM* vm, TutExpr* exp)
{
	assert(exp);

	switch (exp->type)
	{
		case TUT_EXPR_STRUCT_DEF:
		case TUT_EXPR_VAR:
		{
			// nothing
		} break;

		case TUT_EXPR_BLOCK:
		{
			TUT_LIST_EACH(node, exp->blockList)
				CompileStatement(module, vm, node->value);
		} break;

		case TUT_EXPR_IF:
		{
			CompileValue(module, vm, exp->ifx.cond);
			int32_t patchLoc = Tut_EmitGoto(vm, TUT_TRUE, 0);

			CompileStatement(module, vm, exp->ifx.body);
			int32_t exitPatchLoc = Tut_EmitGoto(vm, TUT_FALSE, 0);
			
			Tut_PatchGoto(vm, patchLoc, vm->codeSize);

			if (exp->ifx.alt)
				CompileStatement(module, vm, exp->ifx.alt);

			Tut_PatchGoto(vm, exitPatchLoc, vm->codeSize);
		} break;

		case TUT_EXPR_WHILE:
		{
			int continueLoc = vm->codeSize;
			
			CompileValue(module, vm, exp->whilex.cond);
			int32_t patchLoc = Tut_EmitGoto(vm, TUT_TRUE, 0);

			CompileStatement(module, vm, exp->whilex.body);
			Tut_EmitGoto(vm, TUT_FALSE, continueLoc);

			Tut_PatchGoto(vm, patchLoc, vm->codeSize);
		} break;

		case TUT_EXPR_FUNC:
		{
			Tut_EmitFunctionEntryPoint(vm);

			// Make space for each locals
			int totalLocalSize = 0;
			TUT_LIST_EACH(node, exp->funcx.decl->locals)
			{
				TutVarDecl* decl = node->value;
				totalLocalSize += Tut_GetTypetagSize(decl->typetag);
			}
			if (totalLocalSize > 0)
				Tut_EmitPush(vm, totalLocalSize);

			CompileStatement(module, vm, exp->funcx.body);
			
			Tut_EmitOp(vm, TUT_OP_RET);
		} break;

		case TUT_EXPR_CALL:
		{
			CompileCall(module, vm, exp, TUT_TRUE);
		} break;

		case TUT_EXPR_RETURN:
		{
			if (exp->retx.value)
			{
				assert(exp->retx.value->typetag);

				CompileValue(module, vm, exp->retx.value);
				Tut_EmitRetval(vm, Tut_GetTypetagSize(exp->retx.value->typetag));
			}
			else
				Tut_EmitOp(vm, TUT_OP_RET);
		} break;

		case TUT_EXPR_BIN:
		{
			if (exp->binx.op != TUT_TOK_ASSIGN)
				CompilerError(exp, "Found expression when expecting statement.\n");

			CompileValue(module, vm, exp->binx.rhs);
			CompileAssign(module, vm, exp->binx.lhs);
		} break;

		default:
			CompilerError(exp, "Found expression when expecting statement.\n");
			break;
	}
}

void Tut_CompileModule(TutModule* module, TutVM* vm)
{
	TutFuncDecl* decl = Tut_GetFuncDecl(module->symbolTable, "_main");
	if (!decl)
		Tut_ErrorExit("Module '%s' has no '_main' function.\n", module->name);

	// Goto _main
	int32_t patchLoc = Tut_EmitGoto(vm, TUT_FALSE, 0);

	TutList allModules;

	Tut_InitList(&allModules);

	TUT_LIST_EACH(node, module->importedModules)
		Tut_ListAppend(&allModules, node->value);

	Tut_ListAppend(&allModules, module);

	TUT_LIST_EACH(node, allModules)
	{
		TutModule* mod = node->value;

		FinalizeTypes(mod);
		ResolveVariableIndices(mod);

		TUT_LIST_EACH(node, mod->exprList)
			ResolveSymbols(mod, node->value);

		TUT_LIST_EACH(node, mod->exprList)
			ResolveTypes(mod, node->value);

		TUT_LIST_EACH(node, mod->exprList)
			CompileStatement(mod, vm, node->value);
	}

	int32_t pc = TUT_ARRAY_GET_VALUE(&vm->functionPcs, decl->index, int32_t);
	Tut_PatchGoto(vm, patchLoc, pc);

	int numExterns = 0;
	TUT_LIST_EACH(node, module->symbolTable->functions)
	{
		TutFuncDecl* decl = node->value;
		if (decl->type == TUT_FUNC_DECL_EXTERN)
			++numExterns;
	}

	void* value = NULL;

	Tut_ArrayResize(&vm->externs, numExterns, &value);
	Tut_ArrayResize(&vm->externNames, numExterns, &value);
}

void Tut_SetCompilerFlag(Tut_CompilerFlag flag, const char* value)
{
	Flags[flag] = value;
}

void Tut_BindExternFindIndex(TutModule* module, TutVM* vm, const char* name, TutVMExternFunction fn)
{
	TutFuncDecl* decl = Tut_GetFuncDecl(module->symbolTable, name);
	if (decl && decl->type == TUT_FUNC_DECL_EXTERN && decl->index >= 0)
		Tut_BindExtern(vm, decl->index, name, fn);
}