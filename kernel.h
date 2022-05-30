/**
 * @brief fat file system meta data
 *
 */

#ifndef FATFS_H_
#define FATFS_H_

#include "util.h"
// #include "disk.h"

// blk state
constexpr u8_t FAT_FREE = u8_t(0x00);
constexpr u8_t FAT_EOF = u8_t(0xff);

// attribute bit
constexpr u8_t ATTR_DIRECTORY = 0x10;
// constexpr u8_t ATTR_READ_ONLY = 0x1;
// constexpr u8_t ATTR_HIDDEN = 0x2;

// file type
constexpr u8_t TYPE_FILE = 0x0;
constexpr u8_t TYPE_DIR = 0x1;

// close mode
constexpr u8_t CLOSE_SYNC = 0x0;
constexpr u8_t CLOSE_NO_SYNC = 0x1;

// sync mode
constexpr u8_t SYNC_HEAD = 0x1;
constexpr u8_t SYNC_FCB = 0x2;
constexpr u8_t SYNC_STRONG = SYNC_FCB | SYNC_HEAD;

// constexpr u8_t TYPE_DIR = 0x1;

// dos boot record / more formally -> os boot record(OBR)
typedef struct DBR
{
    u8_t BS_FSSig[10];
    u8_t BPB_BytsPerBlk; // no use, eq to DISK_BytsPerBlk
    u8_t BPB_TotBlk;
    u8_t BPB_RsrvSz;
    u8_t BPB_FATSz;
    u8_t BPB_RootSz;
    u8_t BPB_DirEntSz; // byte
} dbr_t;

typedef u8_t fatEnt_t;

typedef struct DirEnt
{
    char filename[6];
    u8_t attribute;
    u8_t first;
} dirEnt_t;

// file control block
typedef struct FCB
{
    char filename[6] = "";
    u8_t first = FAT_EOF;
    u8_t *buf = nullptr;
    int blkNum;
    // addr <- curBlkId || offset
    int curBlkId;
    int offset;
} fcb_t;

typedef struct DCB
{
    u8_t dirname[6] = "";
    u8_t first = FAT_EOF;
    dirEnt_t curDir[8];
    u8_t blkSz;     // dir size in blk
    u8_t maxEntNum; // dirEntNum * blkSz
} dcb_t;

// boot the file system
void fs_boot();
void fs_shutdown();

// syscall about file
//
int fs_create(const char *filename, u8_t filetype);
int fs_delete(const char *filename);

int fs_open(const char *filename);
void fs_close(int fd);

// deprecated ... unused currently
void fs_sync(u8_t mode);
void fs_fcb_sync(int fd);

// int fs_read(int fd, void *buffer, int nbytes);
// int fs_write(int fd, void *buffer, int nbytes);

// one read and one write
int fs_read(int fd, void *buffer, int nbytes);
int fs_write(int fd, void *buffer, int nbytes);

int fs_ls();
int fs_cd(const char *dirname);

#endif // FATFS_H_

// using namespace std;
// cout << "TinyFAT - File System (Simulation), version 0.0.1-debug (x86_64-pc-whatever-gnu)" << endl;
// cout << "These shell commands are defined internally." << endl;
// cout << "**********************Commands List************************\n";
// cout << "mkdir ls touch cd rm open close read wirte" << endl;
// cout << "Type \"help\"  to see more infomation." << endl;

// if (inRoot) // rename
// {
//     int rtId = 0;
//     while (rtId < rootEntNum)
//     {
//         if (!strcmp(rootDir[rtId].filename, filename) && (filetype))
//         {
//             log("no rootEnt\n");
//             return -1;
//         }
//     }
// }
// else // not in root
// {
//     int dirId = 0;
//     if (!strcmp(dcb->curDir[dirId].filename, filename))
//     {
//         log("no dirEnt\n");
//         return -1;
//     }