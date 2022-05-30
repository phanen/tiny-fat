// #include "kernel.h"
// #include "disk.h"
// #include "util.h"

// // flush all buffer into disk
// void fs_sync(u8_t mode = SYNC_STRONG)
// {
//     if (mode & SYNC_HEAD)
//     {
//         blk_t tmp;
//         u8_t *ptr = (u8_t *)&tmp;

//         // store the fat into disk
//         u8_t *src = (u8_t *)fat;
//         int bid = fatBaseBid;
//         for (; bid < rootBaseBid; bid++, src += dbr->BPB_BytsPerBlk)
//             disk_bwrite((blk_t *)src, bid);

//         // load the root-dir into mem
//         src = (u8_t *)rootDir;
//         bid = rootBaseBid;
//         for (; bid < datBaseBid; bid++, src += dbr->BPB_BytsPerBlk)
//             disk_bwrite((blk_t *)src, bid);
//     }

//     if (mode == SYNC_FCB)
//     {
//         // flush the fcb
//         for (int fcbId = 0; fcbId < fcbNum; fcbId++)
//         {
//             if (fcbs[fcbId].filename[0] != '\0')
//                 fs_fcb_sync(fcbId);
//         }
//     }
// }

// void fs_fcb_sync(int fd)
// {
//     // defensive
//     if (fd < 0 || fcbs[fd].filename[0] == '\0')
//     {
//         log("fd not availible\n");
//         return;
//     }

//     // flush the mem
//     int fatId = fcbs[fd].first;
//     blk_t *buf = (blk_t *)fcbs[fd].buf;
//     while (fatId != FAT_EOF)
//     {
//         disk_bwrite(buf, fatId);
//         fatId = fat[fatId];
//         buf += 1;
//     }
// }