#ifndef TUT_OPCODES_H
#define TUT_OPCODES_H

typedef enum
{
	TUT_OP_PUSH_TRUE,
	TUT_OP_PUSH_FALSE,
	TUT_OP_PUSH_INT,
	TUT_OP_PUSH_FLOAT,
	TUT_OP_PUSH_CSTR,

	TUT_OP_PUSHN,			// push n (uint16) uninitialized objects onto stack (i.e increment stack pointer)
	TUT_OP_PUSH1,			// push 1 ...
	TUT_OP_POPN,			// pop n (uint16) objects off of the stack (decrement stack pointer, usually to discard function return value)
	TUT_OP_POP1,			// pop 1 object off the stack
	
	TUT_OP_MOVEN,			// move n (uint16) objects down m (uint16) spaces in the stack
							// ex. stack = [10, 20, 30] -> TUT_OP_MOVEN 2, 1 -> stack = [20, 30]
	TUT_OP_MOVE1,			// just like moven except n = 1

	TUT_OP_GETGLOBALN,		// push n (uint16) globals starting at index m (int32) onto the stack in order
	TUT_OP_GETGLOBAL1,		// push 1 global from index m (int32) onto the stack

	TUT_OP_SETGLOBALN,		// pop n (uint16) locals and assign them to values starting at index m (int32) in reverse order
							// (i.e first value popped is at the highest index and so on)
	TUT_OP_SETGLOBAL1,		// push 1 local ...

	TUT_OP_GETLOCALN,
	TUT_OP_GETLOCAL1,

	TUT_OP_SETLOCALN,
	TUT_OP_SETLOCAL1,
	
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

	TUT_OP_RETVALN,
	TUT_OP_RETVAL1,

	TUT_OP_GOTO,
	TUT_OP_GOTOFALSE,
	
	TUT_OP_HALT
} TutOpcode;

#endif
