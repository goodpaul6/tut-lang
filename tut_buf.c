#include "tut_buf.h"

#define READ_VALUE_FUNCTION(type, funcName) static type funcName(const uint8_t* buf, uint32_t pos) \
{ \
	assert(pos >= 0); \
	\
	union \
	{ \
		uint8_t bytes[sizeof(type)]; \
		type value; \
	} conv; \
	\
	memcpy(conv.bytes, &buf[pos], sizeof(conv.bytes)); \
	return conv.value; \
}

READ_VALUE_FUNCTION(uint8_t, Tut_ReadUint8)
READ_VALUE_FUNCTION(uint16_t, Tut_ReadUint16)
READ_VALUE_FUNCTION(uint32_t, Tut_ReadUint32)
READ_VALUE_FUNCTION(int8_t, Tut_ReadInt8)
READ_VALUE_FUNCTION(int16_t, Tut_ReadInt16)
READ_VALUE_FUNCTION(int32_t, Tut_ReadInt32)
READ_VALUE_FUNCTION(float, Tut_ReadFloat)

#undef READ_VALUE_FUNCTION

#define WRITE_VALUE_FUNCTION(type, funcName) static void funcName(uint8_t* buf, uint32_t pos, type value) \
{ \
	assert(pos >= 0); \
	\
	union \
	{ \
		uint8_t bytes[sizeof(type)]; \
		type value; \
	} conv; \
	conv.value = value; \
	memcpy(&buf[pos], conv.bytes, sizeof(conv.bytes)); \
}

WRITE_VALUE_FUNCTION(uint8_t, Tut_WriteUint8)
WRITE_VALUE_FUNCTION(uint16_t, Tut_WriteUint16)
WRITE_VALUE_FUNCTION(uint32_t, Tut_WriteUint32)
WRITE_VALUE_FUNCTION(int8_t, Tut_WriteInt8)
WRITE_VALUE_FUNCTION(int16_t, Tut_WriteInt16)
WRITE_VALUE_FUNCTION(int32_t, Tut_WriteInt32)
WRITE_VALUE_FUNCTION(float, Tut_WriteFloat)

#undef WRITE_VALUE_FUNCTION

