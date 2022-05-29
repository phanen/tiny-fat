/**
 * @file disk.cc
 * @author phanen
 * @brief simulate disk
 * @date 2022-05-28
 *
 * @copyright Copyright (c) 2022. phanen
 *
 */

#include "disk.h"
#include "util.h"

#include <string.h>
#include <stdio.h>

static u8_t Disk[DISK_MAXLEN];

void disk_bread(blk_t *data, int blk_id)
{
    memcpy(data, (blk_t *)Disk + blk_id, sizeof(blk_t));
}

void disk_bwrite(blk_t *data, int blk_id)
{
    memcpy((blk_t *)Disk + blk_id, data, sizeof(blk_t));
}
