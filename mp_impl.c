
#include "mp_const.h"
#include "mp_frwd.h"
#include "mp_types.h"

MP_ErrorCode MP_errno = MP_EC_NONE;
pthread_mutex_t errorMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t allocMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t poolMutex = PTHREAD_MUTEX_INITIALIZER;

void
LockMutex(pthread_mutex_t *pMutex) {
    ReturnIfFalse(pMutex != NULL_MUTEX_PTR);
    int32_t res = pthread_mutex_lock(pMutex);
    AbortIfFalse(res == 0);
}

void
UnlockMutex(pthread_mutex_t *pMutex) {
    ReturnIfFalse(pMutex != NULL_MUTEX_PTR);
    int32_t res = pthread_mutex_unlock(pMutex);
    AbortIfFalse(res == 0);
}

void
SetError(MP_ErrorCode ec) {
    LockMutex(&errorMutex);
    if (((int32_t) ec == (int32_t) MP_errno)) {
        goto ret;
    }
    bool valid = ((int32_t) ec >= (int32_t) MP_EC_NONE) &&
            ((int32_t) ec <= (int32_t) MP_EC_OTHER);
    if (!valid) {
        LogError(false, "Invalid errcode %d", (int32_t) ec);
    }
    MP_errno = valid ? ec : MP_EC_OTHER;
ret:
    UnlockMutex(&errorMutex);
}

MP_Result
AllocByteArray(MP_Byte** ppArray, size_t nBytes) {
    LockMutex(&allocMutex);
    MP_ErrorCode ec = MP_EC_NONE;
    if (!(nBytes > 0)) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "AllocByteArray(): INVARG");
        goto ret;
    }
    void* p = (void *) calloc(nBytes, sizeof (MP_Byte));
    AbortIfFalse(p != NULL_VOID_PTR);
    *ppArray = (MP_Byte*) p;
ret:
    SetError(ec);
    UnlockMutex(&allocMutex);
    return ((int32_t) ec == (int32_t) MP_EC_NONE) ? MP_OK : MP_FAILED;
}

MP_Result
FreeByteArray(MP_Byte** ppArray) {
    LockMutex(&allocMutex);
    MP_ErrorCode ec = MP_EC_NONE;
    if (*ppArray == NULL_BYTE_PTR) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "FreeByteArray(): INVARG");
        goto ret;
    }
    free((void*) *ppArray);
    *ppArray = NULL_BYTE_PTR;
ret:
    SetError(ec);
    UnlockMutex(&allocMutex);
    return ((int32_t) ec == (int32_t) MP_EC_NONE) ? MP_OK : MP_FAILED;
}

static inline
size_t
AlignUp(size_t n) {
    if (n % ALIGNMENT != 0) {
        n += ALIGNMENT - n % ALIGNMENT;
    }
    return n;
}

struct BlockHeader*
CreateBlock(void) {
    size_t hdrSize = AlignUp(sizeof (struct BlockHeader));
    size_t totalSize = hdrSize + MAX_BLOCK_SIZE;
    MP_Byte* pMem;
    MP_Result res = AllocByteArray((MP_Byte**) & pMem, totalSize);
    ReturnValIfFalse((res == MP_OK), NULL_BLOCK_PTR);
    struct BlockHeader* pBlk = (struct BlockHeader*) pMem;
    pBlk->pNext = NULL_BLOCK_PTR;
    pBlk->pStart = pMem + hdrSize;
    pBlk->pAvail = pBlk->pStart;
    pBlk->pEnd = pMem + totalSize;
#if TEST_MODE
    Trace("%s", "New block created");
#endif
    return pBlk;
}

MP_PoolHandle*
MP_CreatePool(void) {
    MP_Byte* pMem;
    MP_Result res = AllocByteArray((MP_Byte**) & pMem, sizeof (struct PoolHeader));
    ReturnValIfFalse((res == MP_OK), NULL_POOL_HANDLE_PTR);
    struct PoolHeader *pPool = (struct PoolHeader*) pMem;
    struct BlockHeader *pBlk = CreateBlock();
    ReturnValIfFalse((pBlk != NULL_BLOCK_PTR), NULL_POOL_HANDLE_PTR);
    pPool->pFirst = pBlk;
    pPool->pCurrent = pBlk;
    pPool->count = 1;
#if TEST_MODE
    Trace("%s", "New pool created");
#endif
    return (MP_PoolHandle*) pPool;
}

extern MP_Result
MP_DestroyPool(MP_PoolHandle** ppHnd) {
    LockMutex(&poolMutex);
    MP_ErrorCode ec = MP_EC_NONE;
    if (*ppHnd == NULL_POOL_HANDLE_PTR) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_DestroyPool(): INVARG");
        goto ret;
    }
    struct PoolHeader* pPool = (struct PoolHeader*) *ppHnd;
    if (pPool->pFirst == NULL_BLOCK_PTR) {
        ec = MP_EC_NULLPTR;
        LogError(false, "%s", "MP_DestroyPool(): NULLPTR");
        goto ret;
    }
    if (pPool->count == 0) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_DestroyPool(): INVARG");
        goto ret;
    }
    MP_Result res;
    struct BlockHeader* pBlk;
    while (pPool->pFirst != NULL_BLOCK_PTR) {
        pBlk = pPool->pFirst;
        pPool->pFirst = (pPool->pFirst)->pNext;
        res = FreeByteArray((MP_Byte**) & pBlk);
        if (res == MP_FAILED) {
            ec = MP_errno;
            LogError(false, "%s", "DestroyPool(): MP_FAILED");
            goto ret;
        }
        pPool->count--;
#if TEST_MODE
        Trace("%s", "Block deleted");
#endif
    }
    res = FreeByteArray((MP_Byte**) & pPool);
    if (res == MP_FAILED) {
        ec = MP_errno;
        LogError(false, "%s", "DestroyPool(): MP_FAILED");
        goto ret;
    }
#if TEST_MODE
    Trace("%s", "Pool deleted");
#endif
    pPool = NULL_POOL_PTR;
ret:
    SetError(ec);
    UnlockMutex(&poolMutex);
    return ((int32_t) ec == (int32_t) MP_EC_NONE) ? MP_OK : MP_FAILED;
}

extern MP_Byte*
MP_AllocFromPool(MP_PoolHandle* pHnd, size_t nBytes) {
    LockMutex(&poolMutex);
    MP_Byte* pMem = NULL_BYTE_PTR;
    MP_ErrorCode ec = MP_EC_NONE;
    if (pHnd == NULL_POOL_HANDLE_PTR) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_AllocFromPool(): MP_EC_INVARG");
        goto ret;
    }
    if ((nBytes > MAX_ALLOC_SIZE) || (nBytes < 1)) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_AllocFromPool(): MP_EC_INVARG");
        goto ret;
    }
    struct PoolHeader* pPool = (struct PoolHeader*) pHnd;
    if (pPool->pCurrent == NULL_BLOCK_PTR) {
        ec = MP_EC_NULLPTR;
        LogError(false, "%s", "MP_AllocFromPool(): MP_EC_NULLPTR");
        goto ret;
    }
    nBytes = AlignUp(nBytes);
    struct BlockHeader* pBlk;
    pBlk = pPool->pCurrent;
    if ((size_t) (pBlk->pEnd - pBlk->pAvail) < nBytes) {
        if (pBlk->pNext != NULL_BLOCK_PTR) {
            pBlk = pBlk->pNext;
            pBlk->pAvail = pBlk->pStart;
            pPool->pCurrent = pBlk;
        } else {
            if (pPool->count == MAX_BLOCK_COUNT) {
                ec = MP_EC_LIMIT_REACHED;
                LogError(false, "%s", "MP_AllocFromPool(): MP_EC_LIMIT_REACHED");
                goto ret;
            }
            pBlk = CreateBlock();
            if (pBlk == NULL_BLOCK_PTR) {
                ec = MP_EC_NULLPTR;
                LogError(false, "%s", "MP_AllocFromPool(): MP_EC_NULLPTR");
                goto ret;
            }
            (pPool->pCurrent)->pNext = pBlk;
            pPool->pCurrent = (pPool->pCurrent)->pNext;
            pPool->count++;
        }
    }
    pMem = pBlk->pAvail;
    pBlk->pAvail += nBytes;
ret:
    SetError(ec);
    UnlockMutex(&poolMutex);
    return pMem;
}

extern MP_Byte*
MP_ReallocFromPool(MP_PoolHandle* pHnd,
        MP_Byte* pOldMem,
        size_t oldSize,
        size_t newSize) {
    LockMutex(&poolMutex);
    MP_Byte* pNewMem = NULL_BYTE_PTR;
    MP_ErrorCode ec = MP_EC_NONE;
    if (pHnd == NULL_POOL_HANDLE_PTR) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_ReallocFromPool(): MP_EC_INVARG");
        goto ret;
    }
    if (pOldMem == NULL_BYTE_PTR) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_ReallocFromPool(): MP_EC_INVARG");
        goto ret;
    }
    if ((newSize > MAX_ALLOC_SIZE) || (newSize < 1)) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_ReallocFromPool(): MP_EC_INVARG");
        goto ret;
    }
    pNewMem = MP_AllocFromPool(pHnd, newSize);
    if (pNewMem == NULL_BYTE_PTR) {
        ec = MP_EC_NULLPTR;
        LogError(false, "%s", "MP_ReallocFromPool(): MP_EC_NULLPTR");
        goto ret;
    }
    /*
    for (int32_t i = 0; i < oldSize; i++) {
        *(pNewMem + i) = *(pOldMem + i); // pNewMem[i] = pOldMem[i]
    }*/
    memcpy(pNewMem, pOldMem, oldSize);
ret:
    SetError(ec);
    UnlockMutex(&poolMutex);
    return pNewMem;
}

extern MP_Result
MP_ResetPool(MP_PoolHandle* pHnd) {
    LockMutex(&poolMutex);
    MP_ErrorCode ec = MP_EC_NONE;
    if (pHnd == NULL_POOL_HANDLE_PTR) {
        ec = MP_EC_INVARG;
        LogError(false, "%s", "MP_ResetPool(): MP_EC_INVARG");
        goto ret;
    }
    struct PoolHeader* pPool = (struct PoolHeader*) pHnd;
    if ((pPool->pCurrent == NULL_BLOCK_PTR) || (pPool->pFirst == NULL_BLOCK_PTR)) {
        ec = MP_EC_NULLPTR;
        LogError(false, "%s", "MP_ResetPool(): MP_EC_NULLPTR");
        goto ret;
    }
    //
    struct BlockHeader* pBlk;
    for (pBlk = pPool->pFirst; pBlk != NULL_BLOCK_PTR; pBlk = pBlk->pNext) {
        memset(pBlk->pStart, 0, ((size_t) (pBlk->pEnd - pBlk->pStart)) + 1);
        pBlk->pAvail = pBlk->pStart;
    }
    //
    pPool->pCurrent = pPool->pFirst;
#if TEST_MODE
    Trace("%s", "Pool reset");
#endif
ret:
    SetError(ec);
    UnlockMutex(&poolMutex);
    return ((int32_t) ec == (int32_t) MP_EC_NONE) ? MP_OK : MP_FAILED;
}
