#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "mempool.h"

#define POINT_SIZE sizeof(Point)
#define NPOINTS 100

typedef struct {
    int32_t x, y, z;
} Point;

static void
printerr(void);

int main(void) {
    MP_PoolHandle* hndp = MP_CreatePool();
    if (hndp == (MP_PoolHandle*) NULL) {
        printerr();
    }
    static Point * points[NPOINTS];
    for (int32_t i = 0; i < NPOINTS; i++) {
        points[i] = (Point*) MP_AllocFromPool(hndp, POINT_SIZE);
        if (points[i] == (Point*) NULL) {
            printerr();
        }
        points[i]->x = i;
        points[i]->y = i + 1;
        points[i]->z = i + 2;
    }
    for (int32_t i = 0; i < NPOINTS; i++) {
        printf("%d %d %d\n", points[i]->x, points[i]->y, points[i]->z);
    }
    if (MP_ResetPool(hndp) == MP_FAILED) {
        printerr();
    }
    static Point * points2[NPOINTS];
    for (int32_t i = 0; i < NPOINTS; i++) {
        points2[i] = (Point*) MP_AllocFromPool(hndp, POINT_SIZE);
        if (points2[i] == (Point*) NULL) {
            printerr();
        }
        points2[i]->x = i;
        points2[i]->y = i + 1;
        points2[i]->z = i + 2;
    }
    for (int32_t i = 0; i < NPOINTS; i++) {
        printf("%d %d %d\n", points2[i]->x, points2[i]->y, points2[i]->z);
    }
    if (MP_DestroyPool(&hndp) == MP_FAILED) {
        printerr();
    }
    return 0;
}

static void
printerr(void) {
    printf("errcode=%d\n", MP_errno);
    exit(EXIT_FAILURE);
}
