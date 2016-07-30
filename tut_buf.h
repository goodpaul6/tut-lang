#ifndef TUT_BUF_H
#define TUT_BUF_H

uint8_t Tut_ReadUint8(const uint8_t* buf, uint32_t pos);
uint16_t Tut_ReadUint16(const uint8_t* buf, uint32_t pos);
uint32_t Tut_ReadUint32(const uint8_t* buf, uint32_t pos);
int8_t Tut_ReadInt8(const uint8_t* buf, uint32_t pos);
int16_t Tut_ReadInt16(const uint8_t* buf, uint32_t pos);
int32_t Tut_ReadInt32(const uint8_t* buf, uint32_t pos);
float Tut_ReadFloat(const uint8_t* buf, uint32_t pos);

void Tut_WriteUint8(uint8_t* buf, uint32_t pos, uint8_t value);
void Tut_WriteUint16(uint8_t* buf, uint32_t pos, uint16_t value);
void Tut_WriteUint32(uint8_t* buf, uint32_t pos, uint32_t value);
void Tut_WriteInt8(uint8_t* buf, uint32_t pos, int8_t value);
void Tut_WriteInt16(uint8_t* buf, uint32_t pos, int16_t value);
void Tut_WriteInt32(uint8_t* buf, uint32_t pos, int32_t value);
void Tut_WriteFloat(uint8_t* buf, uint32_t pos, float value);

#endif
