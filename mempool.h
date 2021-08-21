#if !defined(MEMPOOL_H)
#define MEMPOOL_H

typedef uint8_t MP_Byte;
typedef union PoolHandle MP_PoolHandle;
typedef enum Result MP_Result;
typedef enum ErrorCode MP_ErrorCode;

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

extern MP_ErrorCode MP_errno;

extern MP_PoolHandle*
MP_CreatePool (void);
extern MP_Result
MP_DestroyPool (MP_PoolHandle**);
extern MP_Byte*
MP_AllocFromPool (MP_PoolHandle*, size_t);
extern MP_Byte*
MP_ReallocFromPool (MP_PoolHandle*, MP_Byte*, size_t, size_t);
extern MP_Result
MP_ResetPool (MP_PoolHandle*);

#endif
