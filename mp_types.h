#ifndef MP_TYPES_H
#define MP_TYPES_H

enum Result
  {
   MP_OK,
   MP_FAILED
  };

enum ErrorCode
  {
   MP_EC_NONE,
   MP_EC_INVARG,
   MP_EC_NULLPTR,
   MP_EC_LIMIT_REACHED,
   MP_EC_OTHER
  };

struct BlockHeader
{
  struct BlockHeader *pNext;
  MP_Byte *pStart;
  MP_Byte *pAvail;
  MP_Byte *pEnd;
};

struct PoolHeader
{
  struct BlockHeader *pFirst;
  struct BlockHeader *pCurrent;
  int32_t count;
};

union PoolHandle
{
  MP_Byte flag;
};

#endif
