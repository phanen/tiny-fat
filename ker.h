
#include "disk.h" // like disk driver
#include "kernel.h"

#include <string.h>
#include <stdio.h>

// always open
static dbr_t *dbr;
static fatEnt_t *fat; // should align with the one in disk
static dirEnt_t *rootDir;

// sometimes open
// u8_t fcb_map;
static fcb_t *fcbs;
static dcb_t *dcb = new dcb_t;
static bool inRoot;

static int rootEntNum;
static int dirEntNum;
static int fatNum;      // 96
static int fatBaseBid;  // 0 + 1 = 1
static int rootBaseBid; // 0 + 1 + 3 = 4
static int datBaseBid;  // 0 + 1 + 3 + 2 = 6
static int datBlkNum;   // 80 - 6 = 74

static int fcbNum = 8;

// boot the file system
void fs_boot()
{
    blk_t tmp;

    // load dbr into mem
    disk_bread(&tmp, 0);
    u8_t *ptr = (u8_t *)&tmp;
    dbr = new dbr_t;
    strcpy((char *)dbr->BS_FSSig, (char *)(ptr += 3)); // '\0' is part of sig
    ptr += 10;
    dbr->BPB_BytsPerBlk = *ptr++; // no use, eq to DISK_BytsPerBlk
    dbr->BPB_TotBlk = *ptr++;
    dbr->BPB_RsrvSz = *ptr++;
    dbr->BPB_FATSz = *ptr++;
    dbr->BPB_RootSz = *ptr++;
    dbr->BPB_DirEntSz = *ptr++; // byte

    // some base addr info
    fatBaseBid = 0 + dbr->BPB_RsrvSz;
    rootBaseBid = fatBaseBid + dbr->BPB_FATSz;
    datBaseBid = rootBaseBid + dbr->BPB_RootSz;

    // load fat into mem
    fatNum = dbr->BPB_FATSz * dbr->BPB_BytsPerBlk / sizeof(fatEnt_t);
    fat = new fatEnt_t[fatNum]{}; // set-zero
    u8_t *dst = fat;
    int bid = fatBaseBid;
    for (; bid < rootBaseBid; ++bid, dst += dbr->BPB_BytsPerBlk)
        disk_bread((blk_t *)dst, bid);

    // load the root-dir into mem
    dirEntNum = dbr->BPB_BytsPerBlk / dbr->BPB_DirEntSz; // 4
    rootEntNum = dbr->BPB_RootSz * dirEntNum;            // 8
    rootDir = new dirEnt_t[rootEntNum]{};                // set-zero
    dst = (u8_t *)rootDir;
    bid = rootBaseBid;
    for (; bid < datBaseBid; ++bid, dst += dbr->BPB_BytsPerBlk)
        disk_bread((blk_t *)dst, bid);

    datBlkNum = dbr->BPB_TotBlk - datBaseBid;
    fcbs = new fcb_t[fcbNum];
    inRoot = true; // in root dir
    // fcb_map = 0b00000000;
}

void fs_shutdown()
{
    delete dbr;
    delete[] fat;
    delete[] rootDir;

    for (int i = 0; i < fcbNum; i++)
        delete[] fcbs[i].buf;
    delete[] fcbs;
    delete dcb;
}