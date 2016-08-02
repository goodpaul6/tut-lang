#ifndef TUT_OPCODES_H
#define TUT_OPCODES_H

typedef enum
{
	TUT_OP_PUSH_TRUE,
	TUT_OP_PUSH_FALSE,
	TUT_OP_PUSH_INT,
	TUT_OP_PUSH_FLOAT,
	TUT_OP_PUSH_CSTR,

	TUT_OP_PUSH,			// push uninitialized object onto stack (i.e increment stack pointer)
	TUT_OP_POP,				// pop off of the stack (decrement stack pointer, usually to discard function return value)

	TUT_OP_GET_GLOBAL,
	TUT_OP_SET_GLOBAL,

	TUT_OP_GET_LOCAL,
	TUT_OP_SET_LOCAL,
	
	TUT_OP_ADDI,
	TUT_OP_SUBI,
	TUT_OP_MULI,
	TUT_OP_DIVI,
	
	TUT_OP_ADDF,
	TUT_OP_SUBF,
	TUT_OP_MULF,
	TUT_OP_DIVF,
	
	TUT_OP_LAND,
	TUT_OP_LOR,
	TUT_OP_LNOT,

	TUT_OP_ILT,
	TUT_OP_IGT,
	TUT_OP_ILTE,
	TUT_OP_IGTE,
	TUT_OP_IEQ,
	TUT_OP_INEG,

	TUT_OP_FLT,
	TUT_OP_FGT,
	TUT_OP_FLTE,
	TUT_OP_FGTE,
	TUT_OP_FEQ,
	TUT_OP_FNEG,

	TUT_OP_SEQ,

	TUT_OP_CALL,
	TUT_OP_CALL_EXTERN,

	TUT_OP_RET,
	TUT_OP_RETVAL,

	TUT_OP_GOTO,
	TUT_OP_GOTOFALSE,
	
	TUT_OP_HALT
} TutOpcode;

#endif
