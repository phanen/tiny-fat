#include "disk.h" // like disk driver
#include "fatParam.h"

#include <string.h>

/**
 * @brief format the disk
 *  create the boot section (including fs meta data)
 */
void disk_formatter()
{
    // extern u8_t Disk[DISK_MAXLEN];
    // memset(Disk, 0, sizeof(char) * DISK_MAXLEN);

    // create a blk in mem
    blk_t blk0;
    memset(&blk0, 0, sizeof(blk_t));

    u8_t *ptr = (u8_t *)&blk0;
    // jmp to boot code (no use) -- 3b
    int len;
    memcpy(ptr, BS_jmpBoot, len = sizeof(BS_jmpBoot));
    ptr += len;
    // sig -- 10b
    memcpy(ptr, BS_FSSig, len = sizeof(BS_FSSig));
    ptr += len;
    // BPB / DBR -- 5b
    *ptr++ = BPB_BytsPerBlk;
    *ptr++ = BPB_TotBlk;
    *ptr++ = BPB_RsrvSz;
    *ptr++ = BPB_FATSz;
    *ptr++ = BPB_RootSz;
    *ptr++ = BPB_DirEntSz;
    disk_bwrite(&blk0, 0);

    const u8_t FAT_FREE = u8_t(0x00);
    int bid = 0 + BPB_RsrvSz;
    blk_t tmp;
    for (; bid < 0 + BPB_RsrvSz + BPB_FATSz; bid++)
    {
        memset(&tmp, FAT_FREE, sizeof(blk_t));
        disk_bwrite(&tmp, bid);
    }

    for (; bid < BPB_TotBlk; bid++)
    {
        memset(&tmp, 0, sizeof(blk_t));
        disk_bwrite(&tmp, bid);
    }
}