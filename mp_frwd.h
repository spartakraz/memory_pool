#ifndef MP_FRWD_H
#define MP_FRWD_H

enum Result;
enum ErrorCode;
struct BlockHeader;
struct PoolHeader;
union PoolHandle;

typedef uint8_t MP_Byte;
typedef enum Result MP_Result;
typedef enum ErrorCode MP_ErrorCode;
typedef union PoolHandle MP_PoolHandle;

extern MP_ErrorCode MP_errno;
extern pthread_mutex_t errorMutex;
extern pthread_mutex_t allocMutex;
extern pthread_mutex_t poolMutex;

void
LockMutex (pthread_mutex_t *);
void
UnlockMutex (pthread_mutex_t *);

void
SetError (MP_ErrorCode);

MP_Result
AllocByteArray (MP_Byte**, size_t);
MP_Result
FreeByteArray (MP_Byte**);

static inline
size_t AlignUp (size_t);

struct BlockHeader*
CreateBlock (void);

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