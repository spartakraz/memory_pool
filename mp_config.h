#ifndef MP_CONFIG_H
#define MP_CONFIG_H

#if defined(TEST_MODE)

#define MAX_BLOCK_SIZE 512
#define MAX_BLOCK_COUNT 5
#define MAX_ALLOC_SIZE 128
#define ALIGNMENT 4

#else

#define MAX_BLOCK_SIZE sysconf (_SC_PAGESIZE)
#define MAX_BLOCK_COUNT 20
#define MAX_ALLOC_SIZE 512
#define ALIGNMENT 16

#endif

#endif