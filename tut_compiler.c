#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#include "tut_compiler.h"
#include "tut_codegen.h"
#include "tut_opcodes.h"
#include "tut_expr.h"

static void CompilerError(TutExpr* exp, const char* format, ...)
{
	char* lineEnd = strchr(exp->context.lineStart, '\n');
	if (lineEnd)
		fprintf(stderr, "%.*s\n", lineEnd - exp->context.lineStart, exp->context.lineStart);
	else
		fprintf(stderr, "%s\n", exp->context.lineStart);
	fprintf(stderr, "Error (%s, %i): ", exp->context.filename, exp->context.line);

	va_list args;
	
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	exit(1);
}

static void FinalizeTypes(TutModule* module)
{
	TUT_LIST_EACH(node, module->symbolTable.usertypes)
	{
		TutTypetag* t = node->value;
		if (t->type == TUT_TYPETAG_USERTYPE)
		{
			if (!t->user.defined)
				Tut_ErrorExit("Attempted to use undefined type '%s'.\n", t->user.name);

			int count = Tut_GetTypetagCount(t);
			if (count <= 0)
				Tut_ErrorExit("Type '%s' has no members.\n", t->user.name);
		}
	}
}

static void ResolveVariableIndices(TutModule* module)
{
	int globalIndex = 0;
	TUT_LIST_EACH(node, module->symbolTable.globals)
	{
		TutVarDecl* decl = node->value;
		decl->index = globalIndex;

		globalIndex += Tut_GetTypetagCount(decl->typetag);
	}

	TUT_LIST_EACH(node, module->symbolTable.functions)
	{
		TutFuncDecl* decl = node->value;
		if (decl->type == TUT_FUNC_DECL_NORMAL)
		{
			// Calculate the total size of the argument batch
			int totalArgSize = 0;

			TUT_LIST_EACH(argNode, decl->args)
			{
				TutVarDecl* decl = argNode->value;

				totalArgSize += Tut_GetTypetagCount(decl->typetag);
			}

			// The index of each argument is set such that
			// the first argument has the most negative index

			/* Example:
				struct vec2
				{
					x : float;
					y : float;
				}

				// totalArgSize will be 4 (4 floats)
				// a will be assigned index -(4 - 2) - 1 = -3
				// b will be assigned index -(2 - 2) - 1 = -1
				func add(a : vec2, b : vec2) : vec2
				{
					...
				}
			*/
			TUT_LIST_EACH(argNode, decl->args)
			{
				TutVarDecl* decl = argNode->value;
				
				decl->index = -totalArgSize;
				totalArgSize -= Tut_GetTypetagCount(decl->typetag);
			}

			int varIndex = 0;
			TUT_LIST_EACH(varNode, decl->locals)
			{
				TutVarDecl* decl = varNode->value;

				decl->index = varIndex;
				varIndex += Tut_GetTypetagCount(decl->typetag);
			}
		}
	}
}

static void ResolveSymbols(TutModule* module, TutExpr* exp)
{
	assert(exp);

	switch (exp->type)
	{
		case TUT_EXPR_VAR:
		{
			// Already resolved right?
			assert(exp->varx.decl);
		} break;

		case TUT_EXPR_IDENT:
		{
			if (!exp->varx.decl)
			{
				exp->varx.decl = Tut_GetVarDecl(&module->symbolTable, exp->varx.name, 0);
				if (!exp->varx.decl)
				{
					// TODO: Implement function pointers (context-sensitive)
					CompilerError(exp, "Attempted to access undefined variable '%s'\n", exp->varx.name);
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

		case TUT_EXPR_DOT:
		{
			ResolveSymbols(module, exp->dotx.value);
		} break;
		 
		case TUT_EXPR_CALL:
		{
			TutFuncDecl* decl = Tut_GetFuncDecl(&module->symbolTable, exp->callx.func->varx.name);
			if (!decl)
				CompilerError(exp, "Attempted to call non-existent function '%s'\n", exp->callx.func->varx.name);

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

		case TUT_EXPR_STRUCT_DEF:
		{
			// Nothing to do
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
			exp->typetag = Tut_CreatePrimitiveTypetag("str");
		} break;

		case TUT_EXPR_VAR:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
		} break;

		case TUT_EXPR_IDENT:
		{
			assert(exp->varx.decl);
			exp->typetag = exp->varx.decl->typetag;
		} break;

		case TUT_EXPR_UNARY:
		{
			ResolveTypes(module, exp->unaryx.value);
			
			assert(exp->unaryx.op == TUT_TOK_MINUS);

			if (exp->unaryx.op == TUT_TOK_MINUS)
			{
				if (exp->unaryx.value->typetag->type != TUT_TYPETAG_INT && exp->unaryx.value->typetag->type != TUT_TYPETAG_FLOAT)
					CompilerError(exp, "Unary operator '%s' cannot be applied to value of type '%s'.\n", Tut_TokenRepr(exp->unaryx.op), Tut_TypetagRepr(exp->unaryx.value->typetag));

				exp->typetag = exp->unaryx.value->typetag;
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
					if (!Tut_CompareTypes(exp->binx.lhs->typetag, exp->binx.rhs->typetag))
						CompilerError(exp, "Mismatched types in assignment operation (%s, %s).\n", Tut_TypetagRepr(exp->binx.lhs->typetag), Tut_TypetagRepr(exp->binx.rhs->typetag));
				}
				else
				{
					assert(exp->binx.lhs->varx.decl);

					if (!Tut_CompareTypes(exp->binx.lhs->varx.decl->typetag, exp->binx.rhs->typetag))
						CompilerError(exp, "Mismatched types in assignment operation (%s, %s).\n", Tut_TypetagRepr(exp->binx.lhs->typetag), Tut_TypetagRepr(exp->binx.rhs->typetag));
				}
			}
		} break;

		case TUT_EXPR_PAREN:
		{
			ResolveTypes(module, exp->parenExpr);
			exp->typetag = exp->parenExpr->typetag;
		} break;
		
		case TUT_EXPR_DOT:
		{
			ResolveTypes(module, exp->dotx.value);
			assert(exp->dotx.value->typetag);

			if (exp->dotx.value->typetag->type != TUT_TYPETAG_USERTYPE)
				CompilerError(exp, "Type '%s' has no members.\n", Tut_TypetagRepr(exp->dotx.value->typetag));

			TutTypetag* tag = exp->dotx.value->typetag;
			
			TutTypetagMember* found = NULL;

			for (int i = 0; i < tag->user.members.length; ++i)
			{
				TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);
				if (strcmp(mem->name, exp->dotx.memberName) == 0)
				{
					found = mem;
					break;
				}
			}

			if (!found)
				CompilerError(exp, "Type '%s' has no member named '%s'.\n", Tut_TypetagRepr(tag), exp->dotx.memberName);
			
			exp->typetag = found->typetag;
		} break;

		case TUT_EXPR_CALL:
		{
			TutFuncDecl* decl = Tut_GetFuncDecl(&module->symbolTable, exp->callx.func->varx.name);
			assert(decl);

			exp->typetag = decl->returnType;

			TUT_LIST_EACH(node, exp->callx.args)
				ResolveTypes(module, node->value);
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

		case TUT_EXPR_EXTERN:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
		} break;
		
		case TUT_EXPR_RETURN:
		{
			exp->typetag = Tut_CreatePrimitiveTypetag("void");
				
			if (exp->retx.value)
			{
				ResolveTypes(module, exp->retx.value);
				assert(exp->retx.parent);

				if (!Tut_CompareTypes(exp->retx.parent->returnType, exp->retx.value->typetag))
					CompilerError(exp, "Type of value in return statement '%s' does not match return type of enclosing function '%s'\n", Tut_TypetagRepr(exp->retx.value->typetag), Tut_TypetagRepr(exp->retx.parent->returnType));
			}
			else if (exp->retx.parent->returnType->type != TUT_TYPETAG_VOID)
				CompilerError(exp, "Function must return a value.\n");
		} break;
	}
}

static int GetMemberOffset(TutTypetag* tag, const char* memberName)
{
	assert(tag);
	assert(tag->type == TUT_TYPETAG_USERTYPE);

	TutBool found = TUT_FALSE;
	int offset = 0;
	for (int i = 0; i < tag->user.members.length; ++i)
	{
		TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);
		if (strcmp(mem->name, memberName) == 0)
		{
			found = TUT_TRUE;
			break;
		}

		offset += Tut_GetTypetagCount(mem->typetag);
	}

	assert(found);
	return offset;
}

typedef struct
{
	TutVarDecl* decl;
	int offset;
} Lhs;

static TutBool GetBaseLhs(Lhs* lhs, TutExpr* exp, const char* memberName)
{
	assert(exp);
	assert(exp->typetag);
	assert(exp->typetag->type == TUT_TYPETAG_USERTYPE);

	switch (exp->type)
	{
		case TUT_EXPR_VAR:
		case TUT_EXPR_IDENT:
		{
			lhs->decl = exp->varx.decl;
			lhs->offset += GetMemberOffset(exp->typetag, memberName);
			
			return TUT_TRUE;
		} break;

		case TUT_EXPR_DOT:
		{
			if (GetBaseLhs(lhs, exp->dotx.value, exp->dotx.memberName))
			{
				lhs->offset += GetMemberOffset(exp->typetag, memberName);
				return TUT_TRUE;
			}
			
			return TUT_FALSE;
		} break;

		case TUT_EXPR_PAREN:
		{
			return GetBaseLhs(lhs, exp->parenExpr, memberName);
		} break;

		default:
			return TUT_FALSE;
	}
}

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
			Tut_EmitSet(vm, !lhs->varx.decl->parent, lhs->varx.decl->index, Tut_GetTypetagCount(lhs->varx.decl->typetag));
		} break;

		case TUT_EXPR_DOT:
		{
			Lhs lhsData;

			lhsData.decl = NULL;
			lhsData.offset = 0;

			// p.scene.x
			if (!GetBaseLhs(&lhsData, lhs->dotx.value, lhs->dotx.memberName))
				CompilerError(lhs, "Invalid lhs in assignment expression.\n");

			Tut_EmitSet(vm, !lhsData.decl->parent, lhsData.decl->index + lhsData.offset, Tut_GetTypetagCount(lhs->typetag));
		} break;

		default:
			CompilerError(lhs, "Invalid lhs in assignment statement.\n");
			break;
	}
}

static void CompileValue(TutModule* module, TutVM* vm, TutExpr* exp);

static void CompileCall(TutModule* module, TutVM* vm, TutExpr* exp, TutBool discardReturnValue)
{
	TutFuncDecl* decl = Tut_GetFuncDecl(&module->symbolTable, exp->callx.func->varx.name);
	assert(decl);

	if (exp->callx.args.length < decl->args.length)
		CompilerError(exp, "Not enough arguments passed into function '%s'; it takes [at least] %d\n", decl->name, decl->args.length);
	else if (!decl->hasVarargs && exp->callx.args.length > decl->args.length)
		CompilerError(exp, "Too many arguments passed into function '%s'; it takes %d\n", decl->name, decl->args.length);

	TutListNode* argNode = exp->callx.args.head;
	TutListNode* varNode = decl->args.head;
	int i = 1;

	while (argNode && varNode)
	{
		TutExpr* arg = argNode->value;
		TutVarDecl* var = varNode->value;

		assert(arg->typetag);
		assert(var->typetag);

		if (!Tut_CompareTypes(arg->typetag, var->typetag))
			CompilerError(exp, "Argument %d type '%s' does not match expected type '%s'\n", i, Tut_TypetagRepr(arg->typetag), Tut_TypetagRepr(var->typetag));

		argNode = argNode->next;
		varNode = varNode->next;
		++i;
	}

	int totalCount = 0;

	TUT_LIST_EACH(node, exp->callx.args)
	{
		TutExpr* arg = node->value;

		assert(arg->typetag);
		totalCount += Tut_GetTypetagCount(arg->typetag);

		CompileValue(module, vm, node->value);
	}

	Tut_EmitCall(vm, decl->type == TUT_FUNC_DECL_EXTERN, decl->index, (uint16_t)totalCount);

	if (discardReturnValue && decl->returnType->type != TUT_TYPETAG_VOID)
	{
		TutTypetag* ret = decl->returnType;
		Tut_EmitPop(vm, Tut_GetTypetagCount(ret));
	}
}

// old method of getting usertypes (get each member one by one)
#if 0
static void CompileUsertypeAccess(TutModule* module, TutVM* vm, TutTypetag* tag, TutBool isGlobal, int index)
{
	assert(tag->type == TUT_TYPETAG_USERTYPE);
	
	int offset = 0;
	for (int i = 0; i < tag->user.members.length; ++i)
	{
		TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);

		if (mem->typetag->type != TUT_TYPETAG_USERTYPE)
			Tut_EmitGet(vm, isGlobal, index + offset);
		else
			CompileUsertypeAccess(module, vm, mem->typetag, isGlobal, index + offset);
		offset += Tut_GetTypetagCount(mem->typetag);
	}
}
#endif

static void CompileValue(TutModule* module, TutVM* vm, TutExpr* exp)
{
	assert(exp);
	assert(exp->typetag);

	switch (exp->type)
	{
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
			assert(exp->varx.decl);
			assert(exp->varx.decl->index != TUT_VAR_DECL_INDEX_UNDEFINED);

			// BAM! Copy the entire thing at once
			Tut_EmitGet(vm, !exp->varx.decl->parent, exp->varx.decl->index, Tut_GetTypetagCount(exp->varx.decl->typetag));
		} break;

		case TUT_EXPR_UNARY:
		{
			assert(exp->unaryx.op == TUT_TOK_MINUS);

			if (exp->unaryx.op == TUT_TOK_MINUS)
			{
				assert(exp->typetag->type == TUT_TYPETAG_INT || exp->typetag->type == TUT_TYPETAG_FLOAT);

				CompileValue(module, vm, exp->unaryx.value);

				if (exp->typetag->type == TUT_TYPETAG_INT)
					Tut_EmitOp(vm, TUT_OP_INEG);
				else if (exp->typetag->type == TUT_TYPETAG_FLOAT)
					Tut_EmitOp(vm, TUT_OP_FNEG);
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
			int amountToPop = 0;
			int memIndex = -1;

			TutTypetag* tag = exp->dotx.value->typetag;

			for (int i = tag->user.members.length - 1; i >= 0; --i)
			{
				TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);

				if (strcmp(mem->name, exp->dotx.memberName) == 0)
				{
					memIndex = i;
					break;
				}

				amountToPop += Tut_GetTypetagCount(mem->typetag);
			}

			assert(memIndex != -1);

			Tut_EmitPop(vm, amountToPop);
			
			int amountToMove = 0;
			for (int i = memIndex - 1; i >= 0; --i)
			{
				TutTypetagMember* mem = Tut_ArrayGet(&tag->user.members, i);
				amountToMove += Tut_GetTypetagCount(mem->typetag);
			}

			Tut_EmitMove(vm, Tut_GetTypetagCount(exp->typetag), amountToMove);
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
			int totalLocalCount = 0;
			TUT_LIST_EACH(node, exp->funcx.decl->locals)
			{
				TutVarDecl* decl = node->value;
				totalLocalCount += Tut_GetTypetagCount(decl->typetag);
			}
			if (totalLocalCount > 0)
				Tut_EmitPush(vm, totalLocalCount);

			CompileStatement(module, vm, exp->funcx.body);
			
			Tut_EmitOp(vm, TUT_OP_RET);
		} break;

		case TUT_EXPR_EXTERN:
		{
			// Make room for an extern to be bound later
			TutVMExternFunction empty = NULL;
			Tut_ArrayPush(&vm->externs, &empty);
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
				Tut_EmitRetval(vm, Tut_GetTypetagCount(exp->retx.value->typetag));
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
	TutFuncDecl* decl = Tut_GetFuncDecl(&module->symbolTable, "_main");
	if (!decl)
		Tut_ErrorExit("Module '%s' has no '_main' function.\n", module->name);

	// Goto _main
	int32_t patchLoc = Tut_EmitGoto(vm, TUT_FALSE, 0);

	FinalizeTypes(module);
	ResolveVariableIndices(module);

	TUT_LIST_EACH(node, module->exprList)
		ResolveSymbols(module, node->value);

	TUT_LIST_EACH(node, module->exprList)
		ResolveTypes(module, node->value);

	TUT_LIST_EACH(node, module->exprList)
		CompileStatement(module, vm, node->value);

	int32_t pc = TUT_ARRAY_GET_VALUE(&vm->functionPcs, decl->index, int32_t);
	Tut_PatchGoto(vm, patchLoc, pc);
}

void Tut_BindLibrary(TutModule* module, TutVM* vm, uint32_t numExterns, const TutExternDef* externs)
{
	for (uint32_t i = 0; i < numExterns; ++i)
	{
		const TutExternDef* def = &externs[i];

		TutFuncDecl* decl = Tut_GetFuncDecl(&module->symbolTable, def->name);
		if (!decl || decl->type != TUT_FUNC_DECL_EXTERN)
			continue;

		Tut_BindExtern(vm, def->function, decl->index);
	}
}