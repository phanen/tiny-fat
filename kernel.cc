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

static int fat_getId()
{
    int id = 1;
    while (id <= datBlkNum && fat[id] != FAT_FREE)
        ++id;
    return id;
}

// static void root
int fs_create(const char *filename, u8_t filetype)
{
    u8_t attr = (filetype == TYPE_DIR ? ATTR_DIRECTORY : 0x00);
    int fatId;

    if (inRoot)
    { // in root dir

        int rtId = 0;
        while (rtId < rootEntNum && rootDir[rtId].filename[0] != '\0')
            ++rtId;
        if (rtId >= rootEntNum)
        {
            log("no rootEnt\n");
            return -1;
        }

        fatId = fat_getId();
        if (fatId > datBlkNum)
        {
            log("no fat\n");
            return -1;
        }

        fat[fatId] = FAT_EOF;
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
            return -1;
        }

        int fatId = fat_getId();
        if (fatId > datBlkNum)
        {
            log("no fat\n");
            return -1;
        }

        fat[fatId] = FAT_EOF;
        strcpy(dcb->curDir[id].filename, filename);
        dcb->curDir[id].attribute = attr;
        dcb->curDir[id].first = fatId;
    }

    // dir_init
    if (attr & ATTR_DIRECTORY)
    {
        dirEnt_t tmp;
        strcpy(tmp.filename, "..");
        tmp.first = FAT_EOF; // no blk id
        disk_bwrite((blk_t *)&tmp, fatId);
    }
    return 0;
}

void fs_delete(const char *filename)
{
    int fcbId = 0;
    while (fcbId < fcbNum && strcmp(fcbs[fcbId].filename, filename))
        ++fcbId;
    if (fcbId != fcbNum) // the file is still open
        fs_close(fcbId, CLOSE_NO_SYNC);

    int rtId = 0;
    int dirId = 0;
    int fatId = 0;
    if (inRoot) // exist ? find it in cur directory
    {
        while (rtId < rootEntNum &&
               ((rootDir[rtId].attribute & ATTR_DIRECTORY) || // is dir
                strcmp(rootDir[rtId].filename, filename))     // not name
        )
            ++rtId;
        if (rtId == rootEntNum)
        {
            log("no such file\n");
            return;
        }

        // if (rootDir[rtId].attribute & ATTR_DIRECTORY)
        // {
        //     log("cannot del dir");
        //     return;
        // }

        fat[fatId] = rootDir[rtId].first;
        rootDir[rtId].filename[0] = '\0';
        rootDir[rtId].first = FAT_EOF;
        rootDir[rtId].attribute = 0;
    }
    else
    {
        int dirId = 0;
        while (dirId < rootEntNum &&
               ((dcb->curDir[dirId].attribute & ATTR_DIRECTORY) || // is dir
                strcmp(dcb->curDir[dirId].filename, filename))     // not name
        )
            ++dirId;

        if (dirId == dirEntNum)
        {
            log("no such file\n");
            return;
        }

        // if (dcb->curDir[dirId].attribute & ATTR_DIRECTORY)
        // {
        //     log("cannot del dir");
        //     return ;
        // }

        fatId = dcb->curDir[dirId].first;
        dcb->curDir[dirId].filename[0] = '\0';
        dcb->curDir[dirId].attribute = 0;
        dcb->curDir[dirId].first = FAT_EOF;
    }

    // del file from fat
    while (fatId != FAT_EOF)
    {
        u8_t tmp = fat[fatId];
        fat[fatId] = FAT_FREE;
        fatId = tmp;
    }
} // fs_delete

// load file info into fcb and return fcbId as file descriptor
int fs_open(const char *filename)
{
    int rtId = 0;
    int dirId = 0;
    int fatId = 0;

    if (inRoot) // exist ? find it in cur directory
    {
        while (rtId < rootEntNum &&
               ((rootDir[rtId].attribute & ATTR_DIRECTORY) || // is dir
                strcmp(rootDir[rtId].filename, filename))     // not name
        )
            ++rtId;
        if (rtId == rootEntNum)
        {
            log("no such file\n");
            return -1;
        }
    }
    else
    {
        int dirId = 0;
        while (dirId < rootEntNum &&
               ((dcb->curDir[dirId].attribute & ATTR_DIRECTORY) ||
                strcmp(dcb->curDir[dirId].filename, filename)))
            ++dirId;
        if (dirId == dirEntNum)
        {
            log("no such file\n");
            return -1;
        }

        // if (dcb->curDir[dirId].attribute & ATTR_DIRECTORY)
        // {
        //     log("cannot open dir");
        //     return -1;
        // }
    }

    // find a free fcb
    int fcbId = 0;
    while (fcbId < fcbNum && fcbs[fcbId].filename[0] != '\0')
        ++fcbId;
    if (fcbId == fcbNum)
    {
        log("no free fcb\n");
        return -1; // no available mem
    }

    // load the file info into fcb
    strcpy(fcbs[fcbId].filename, filename);
    fcbs[fcbId].first = inRoot ? rootDir[rtId].first : dcb->curDir[dirId].first;
    return fcbId;
}

// flush all buffer into disk
void fs_sync(u8_t mode = SYNC_STRONG)
{
    if (mode & SYNC_HEAD)
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

    if (mode == SYNC_FCB)
    {
        // flush the fcb
        for (int fcbId = 0; fcbId < fcbNum; fcbId++)
        {
            if (fcbs[fcbId].filename[0] != '\0')
                fs_fcb_sync(fcbId);
        }
    }
}

void fs_fcb_sync(int fd)
{
    // defensive
    if (fd < 0 || fcbs[fd].filename[0] == '\0')
    {
        log("fd not availible\n");
        return;
    }

    // flush the mem
    int fatId = fcbs[fd].first;
    blk_t *buf = (blk_t *)fcbs[fd].buf;
    while (fatId != EOF)
    {
        disk_bwrite(buf, fatId);
        fatId = fat[fatId];
        buf += 1;
    }
}

void fs_close(int fd, u8_t mode = CLOSE_SYNC)
{
    if (mode == CLOSE_SYNC)
        fs_sync(fd);

    if (fd < 0)
    {
        log("fd is not available\n");
        return;
    }

    if (fcbs[fd].buf != nullptr)
    {
        delete[] fcbs[fd].buf;
        fcbs[fd].buf = nullptr;
    }

    // make the fcb available
    fcbs[fd].filename[0] = '\0';
}