#include "util.h"
#include "disk.h"

// BPB or DBR
constexpr u8_t BS_jmpBoot[3] = {0xeb, 0x3c, 0x90};
constexpr u8_t BS_FSSig[10] = "tiny-fat";
constexpr u8_t BPB_BytsPerBlk = 32;                       // no use, eq to DISK_BytsPerBlk
constexpr u8_t BPB_TotBlk = DISK_MAXLEN / BPB_BytsPerBlk; // 80
constexpr u8_t BPB_RsrvSz = 1;
constexpr u8_t BPB_FATSz = 3;
constexpr u8_t BPB_RootSz = 2;
constexpr u8_t BPB_DirEntSz = 8; // byte