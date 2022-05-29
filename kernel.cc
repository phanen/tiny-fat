/**
 * @file kernel.cc
 * @author phanen
 * @brief simulate the file system
 * @date 2022-05-29
 *
 * @copyright Copyright (c) 2022. phanen
 *
 */

#include "disk.h" // like disk driver
#include "kernel.h"

#include <string.h>
#include <stdio.h>

// always open
static dbr_t *dbr;
static fatEnt_t *fat; // should align with the one in disk
static dirEnt_t *rootDir;

// sometimes open
static fcb_t *fcbs = new fcb_t[8];
static dcb_t *dcb = new dcb_t;
static bool inRoot;

static int rootEntNum;
static int dirEntNum;
static int fatNum;      // 96
static int fatBaseBid;  // 0 + 1 = 1
static int rootBaseBid; // 0 + 1 + 3 = 4
static int datBaseBid;  // 0 + 1 + 3 + 2 = 6
static int datBlkNum;   // 80 - 6 = 74

static u8_t rootMap;

#define log(info) printf((const char *)(info))

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
    // #include <vector>
    // load the root-dir into mem
    dirEntNum = dbr->BPB_BytsPerBlk / dbr->BPB_DirEntSz; // 4
    rootEntNum = dbr->BPB_RootSz * dirEntNum;            // 8
    rootDir = new dirEnt_t[rootEntNum]{};                // set-zero
    dst = (u8_t *)rootDir;
    bid = rootBaseBid;
    for (; bid < datBaseBid; ++bid, dst += dbr->BPB_BytsPerBlk)
        disk_bread((blk_t *)dst, bid);

    datBlkNum = dbr->BPB_TotBlk - datBaseBid;

    inRoot = true; // in root dir
}

// update header content from mem to disk
void fs_update()
{
    blk_t tmp;
    u8_t *ptr = (u8_t *)&tmp;

    // store the fat into disk
    u8_t *src = (u8_t *)fat;
    int bid = fatBaseBid;
    for (; bid < rootBaseBid; bid++, src += dbr->BPB_BytsPerBlk)
        disk_bwrite((blk_t *)src, bid);

    // load the root-dir into mem
    src = (u8_t *)rootDir;
    bid = rootBaseBid;
    for (; bid < datBaseBid; bid++, src += dbr->BPB_BytsPerBlk)
        disk_bwrite((blk_t *)src, bid);
}

void fs_shutdown()
{
    delete[] dbr;
    delete[] fat;
    delete[] rootDir;

    delete[] fcbs;
    delete[] dcb;
}

static int fat_getId()
{
    int id = 1;
    while (id <= datBlkNum && fat[id] != FREE)
        ++id;
    return id;
}

// static void root
void fs_create(const char *filename, int filetype)
{
    u8_t attr = (filetype == TYPE_DIR ? 0x10 : 0x00);
    int fatId;

    if (inRoot)
    { // in root dir

        int rtId = 0;
        while (rtId < rootEntNum && rootDir[rtId].filename[0] != '\0')
            ++rtId;
        if (rtId >= rootEntNum)
        {
            log("no rootEnt\n");
            return;
        }

        fatId = fat_getId();
        if (fatId > datBlkNum)
        {
            fat[fatId] = FREE;
            log("no fat\n");
            return;
        }

        fat[fatId] = EOF;
        strcpy(rootDir[rtId].filename, filename);
        rootDir[rtId].attribute = attr;
        rootDir[rtId].first = fatId;
    }
    else // not root dir
    {
        int id = 0;
        while (id < dcb->maxEntNum && dcb->curDir[id].filename[0] != '\0')
            ++id;
        if (id == dcb->maxEntNum)
        {
            log("no dirEnt\n");
            return;
        }

        int fatId = fat_getId();
        if (fatId > datBlkNum)
        {
            log("no fat\n");
            return;
        }

        fat[fatId] = EOF;
        strcpy(dcb->curDir[id].filename, filename);
        dcb->curDir[id].attribute = attr;
        dcb->curDir[id].first = fatId;
    }

    // dir_init
    if (attr | ATTR_DIRECTORY)
    {
        dirEnt_t tmp;
        strcpy(tmp.filename, "..");
        tmp.first = 0xff; // father is root
        disk_bwrite((blk_t *)&tmp, fatId);
    }
}

void fs_open()
{
}