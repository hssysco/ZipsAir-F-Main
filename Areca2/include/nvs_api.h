
#ifndef __NVS_H__
#define __NVS_H__

int16_t SetValue(const char *pKey, const char *pStrType, const char *pStrValue);
int16_t GetValue(const char *pKey, const char *pStrType);
int16_t Erase(const char *pKey);
int16_t EraseAll(void);

int16_t SetValueU8(const char *pKey, unsigned char Data);
int16_t SetValueU32(const char *pKey, unsigned int Data);
int16_t SetValueStr(const char *pKey, char *pData);

int16_t GetValueU8(const char *pKey, unsigned char *pData);
int16_t GetValueU32(const char *pKey, unsigned int *pData);
int16_t GetValueStr(const char *pKey, char *pData);

#endif

