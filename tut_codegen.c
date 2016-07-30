#include "tut_codegen.h"
#include "tut_buf.h"
#include "tut_util.h"

void Tut_InitCodegen(TutCodegen* gen)
{
	gen->numFunctions = 0;
	gen->numGlobals = 0;
	
	gen->numIntegers = 0;
	gen->numFloats = 0;
	gen->numStrings = 0;
	
	gen->dataLength = 0;
	gen->codeLength = 0;
}

static void CompileAccess(TutCodegen* gen, TutVarDecl* decl)
{
	if(decl->scope == 0)
	{
		switch(decl->typetag->type)
		{
			case TUT_TYPETAG_BOOL: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_BOOL; break;
			case TUT_TYPETAG_INT: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_BOOL; break;
			case TUT_TYPETAG_FLOAT: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_BOOL; break;
			case TUT_TYPETAG_CSTR: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_CSTR; break;
			case TUT_TYPETAG_STR: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_; break;
			
		}
	}
}

void Tut_CompileValueExpr(TutSymbolTable* table, TutCodegen* gen, TutExpr* exp)
{
	switch(exp->type)
	{
		case TUT_EXPR_INT:
		{
			TutBool found = TUT_FALSE;
			uint32_t offset;
			
			for(int i = 0; i < gen->numIntegers; ++i)
			{
				int32_t value = Tut_ReadInt32(gen->data, gen->integerOffsets[i]);
				if(exp->intVal == value)
				{
					found = TUT_TRUE;
					offset = gen->integerOffsets[i];
					break;
				}
			}
			
			if(!found)
			{
				offset = gen->dataLength;
				gen->integerOffsets[gen->numIntegers++] = offset;
				
				Tut_WriteInt32(gen->data, offset, exp->intVal);
				gen->dataLength += 4;
			}
			
			gen->code[gen->codeLength++] = TUT_OP_PUSH_INT;
			Tut_WriteUint32(gen->code, gen->codeLength, offset);
			gen->codeLength += 4;
		} break;
		
		case TUT_EXPR_FLOAT:
		{
			TutBool found = TUT_FALSE;
			uint32_t offset;
			
			for(int i = 0; i < gen->numFloats; ++i)
			{
				float value = Tut_ReadFloat(gen->data, gen->integerOffsets[i]);
				if(exp->floatVal == value)
				{
					found = TUT_TRUE;
					offset = gen->floatOffsets[i];
					break;
				}
			}
			
			if(!found)
			{
				offset = gen->dataLength;
				gen->floatOffsets[gen->numFloats++] = offset;
				
				Tut_WriteFloat(gen->data, offset, exp->floatVal);
				gen->dataLength += 4;
			}
			
			gen->code[gen->codeLength++] = TUT_OP_PUSH_FLOAT;
			Tut_WriteUint32(gen->code, gen->codeLength, offset);
			gen->codeLength += 4;
		} break;
		
		case TUT_EXPR_STR:
		{
			TutBool found = TUT_FALSE;
			uint32_t offset;
			
			for(int i = 0; i < gen->numStrings; ++i)
			{
				uint32_t length = Tut_ReadUint32(gen->data, gen->stringOffsets[i]);
				const char* data = &gen->data[gen->stringOffsets[i] + 4];
				
				if(Tut_Strncmp(exp->string, data, length) == 0)
				{
					found = TUT_TRUE;
					offset = gen->stringOffsets[i];
					break;
				}
			}
			
			if(!found)
			{
				offset = gen->dataLength;
				gen->stringOffsets[gen->numStrings++] = offset;
				
				uint32_t len = (uint32_t)strlen(exp->string);
				
				Tut_WriteUint32(gen->data, offset, len);
				gen->dataLength += 4;
				for(int i = 0; i < len; ++i)
					gen->data[gen->dataLength + i] = exp->string[i];
				gen->dataLength += len;
			}
			
			gen->code[gen->codeLength++] = TUT_OP_PUSH_CSTR;
			Tut_WriteUint32(gen->code, gen->codeLength, offset);
			gen->codeLength += 4;
		} break;
		
		case TUT_EXPR_VAR:
		{
			if(!exp->varx.decl)
			{	
				exp->varx.decl = Tut_GetVarDecl(table, exp->varx.name, 0); 
				if(!exp->varx.decl)
				{
					// TODO: Produce function pointer
					Tut_ErrorExit("Attempted to access undeclared global variable '%s'\n", exp->varx.name);
				}
			}
			
			if(exp->varx.decl->scope == 0)
			{
				switch(exp->varx.decl->typetag->type)
				{
					case TUT_TYPETAG_BOOL: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_BOOL; break;
					case TUT_TYPETAG_INT: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_INT; break;
					case TUT_TYPETAG_FLOAT: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_FLOAT; break;
					case TUT_TYPETAG_CSTR: gen->code[gen->codeLength++] = TUT_OP_GET_GLOBAL_CSTR; break;
					case TUT_TYPETAG_USERTYPE:
					{
						TUT_LIST_EACH(node, exp->varx.decl->typetag->user.members)
						{
							TutTypetag* mem = node->value;
							
						}
					} break;
				}
			}
		} break;
	}
}

void Tut_DestroyCodegen(TutCodegen* gen);

