/**
 * @file kernel.cc
 * @author phanen
 * @brief simulate the file system
 * @date 2022-05-29
 *
 * @copyright Copyright (c) 2022. phanen
 *
 */

#include "ker.h"
#define log(info) printf((const char *)(info))

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
        tmp.attribute = ATTR_DIRECTORY;
        tmp.first = FAT_EOF; // no blk id
        disk_bwrite((blk_t *)&tmp, fatId);
    }
    return 0;
}

int fs_delete(const char *filename)
{
    int fcbId = 0;
    while (fcbId < fcbNum && strcmp(fcbs[fcbId].filename, filename))
        ++fcbId;
    if (fcbId != fcbNum) // the file is still open
        fs_close(fcbId);
    // fs_close(fcbId, CLOSE_NO_SYNC);

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
            return -1;
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

    // free the fat
    while (fatId != FAT_EOF)
    {
        u8_t tmp = fat[fatId];
        fat[fatId] = FAT_FREE;
        fatId = tmp;
    }
    return 0;
} // fs_delete

// load file info into fcb and return fcbId as file descriptor
int fs_open(const char *filename)
{
    int rtId = 0;
    int dirId = 0;
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
    int fatId = fcbs[fcbId].first;
    int cnt = 1;
    while ((fatId = fat[fatId]) != FAT_EOF)
        ++cnt;
    fcbs[fcbId].blkNum = cnt;
    fcbs[fcbId].curBlkId = 0;
    fcbs[fcbId].offset = 0;
    if (fcbs[fcbId].buf != nullptr)
    {
        delete fcbs[fcbId].buf;
        fcbs[fcbId].buf = nullptr;
    }

    // more ...............
    /**
     *
     *
     */
    return fcbId;
}

void fs_close(int fd)
{
    // if (mode == CLOSE_SYNC)
    //     fs_fcb_sync(fd);

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

// 32 bytes at least
int fs_read(int fd, void *buffer, int nbytes)
{
    // check fd
    if (fd < 0 || fcbs[fd].filename[0] == '\0')
    {
        log("fd not availible\n");
        return -1;
    }

    int fatId = fcbs[fd].first;
    int blkNum = nbytes / dbr->BPB_BytsPerBlk;
    int offset = nbytes % dbr->BPB_BytsPerBlk;

    if (blkNum > fcbs[fd].blkNum)
    {
        blkNum = dbr->BPB_BytsPerBlk;
        offset = 0;
    }

    blk_t *curBuf = (blk_t *)buffer;

    // for intact blk
    int cnt = 0;
    while (cnt < blkNum)
    {
        disk_bread(curBuf++, fatId);
        fatId = fat[fatId];
        ++cnt;
    }

    if (offset)
    {
        blk_t *tmp = new blk_t;
        disk_bread(tmp, fatId);
        memcpy(curBuf, tmp, offset);
        delete tmp;
    }

    return blkNum * dbr->BPB_BytsPerBlk + offset;
}

//
int fs_write(int fd, void *buffer, int nbytes)
{
    // check fd
    if (fd < 0 || fcbs[fd].filename[0] == '\0')
    {
        log("fd not availible\n");
        return -1;
    }

    int blkNum = nbytes / dbr->BPB_BytsPerBlk;
    int offset = nbytes % dbr->BPB_BytsPerBlk;
    if (offset)
        ++blkNum;

    // for intact blk

    int fatId;
    // fat is not enough
    if (fcbs[fd].blkNum < blkNum)
    {
        // locate end
        int nextId = fcbs[fd].first;
        while (nextId != FAT_EOF)
        {
            fatId = nextId;
            nextId = fat[fatId];
        }
        // append new fat
        for (int i = fcbs[fd].blkNum; i < blkNum; i++)
        {
            int newId = fat_getId();
            if (newId == -1)
            {
                log("no fat\n");
                return -1;
            }
            fat[newId] = FAT_EOF;
            fat[fatId] = newId;
            fatId = newId;
        }
        fcbs[fd].blkNum = blkNum;
    }

    blk_t *curBuf = (blk_t *)buffer;
    // for intact blk
    int cnt = 0;
    fatId = fcbs[fd].first;
    if (offset)
    {
        while (cnt < blkNum - 1)
        {
            disk_bwrite(curBuf++, fatId);
            fatId = fat[fatId];
            ++cnt;
        }

        blk_t *tmp = new blk_t{};
        disk_bread(tmp, fatId);
        memcpy(tmp, curBuf, offset);
        disk_bwrite(tmp, fatId);
        delete tmp;
    }
    else
    {
        while (cnt < blkNum)
        {
            disk_bwrite(curBuf++, fatId);
            fatId = fat[fatId];
            ++cnt;
        }
    }
    return blkNum * dbr->BPB_BytsPerBlk;
}

static int calc_sz(int fatId)
{
    if (fatId == FAT_EOF || fatId == FAT_FREE)
        return 0;

    int cnt = 0;
    while (fatId != FAT_EOF)
    {
        fatId = fat[fatId];
        ++cnt;
    }
    return cnt;
}

int fs_ls()
{
    int calc_sz(int);

    if (inRoot)
    {
        for (int rtId = 0; rtId < rootEntNum; ++rtId)
        {
            if (rootDir[rtId].filename[0] != '\0')
            {
                int sz = dbr->BPB_BytsPerBlk * calc_sz(rootDir[rtId].first);
                printf("%  4d \t %6s \t %6s \n",
                       sz,
                       rootDir[rtId].attribute ? "DIR" : "FILE",
                       rootDir[rtId].filename);
            }
        }
        printf("\n");
    }
    else
    {
        for (int dirId = 0; dirId < dcb->maxEntNum; dirId++)
        {
            if (dcb->curDir[dirId].filename[0] != '\0')
            {
                int sz = calc_sz(dcb->curDir[dirId].first);
                printf("%  4d \t %6s \t %6s \n",
                       sz,
                       dcb->curDir[dirId].filename ? "DIR" : "FILE",
                       dcb->curDir[dirId].filename);
            }
        }
        printf("\n");
    }
    return 0;
}

int fs_cd(const char *dirname)
{
}