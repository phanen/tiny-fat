
#ifndef DISK_H_
#define DISK_H_

#include "util.h"

// disk
constexpr u32_t DISK_MAXLEN = 2560;
constexpr u8_t DISK_BytsPerBlk = 32;

// typedef u8_t block_t[DISK_BytsPerBlk];
typedef struct blk_t
{
    u8_t __byteArr[DISK_BytsPerBlk];
} blk_t;

void disk_bread(blk_t *data, int blk_id);
void disk_bwrite(blk_t *data, int blk_id);

#endif // DISK_H_