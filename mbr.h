#ifndef MBR_H_INCLUDED
#define MBR_H_INCLUDED

#include <stdint.h>

#define MBR_BOOT_SIG 0xAA55
#define MBR_PART_COUNT 4

typedef enum
{
    MBR_PART_FREE    = 0x00,
    MBR_PART_MINIX14 = 0x81,
} MBR_PART_TYPE;

#pragma pack(push, 1)

typedef struct
{
    uint8_t uHead;
    uint8_t uCylHiSect;
    uint8_t uCylLo;
} MBR_CHS;

typedef struct
{
    uint8_t uStatus;
    MBR_CHS FirstSectChs;
    MBR_PART_TYPE Type: 8;
    MBR_CHS LastSectChs;
    uint32_t uFirstSectLba;
    uint32_t cSect;
} MBR_PARTITION;

typedef struct
{
    char Bootstrap[446];
    MBR_PARTITION Partitions[MBR_PART_COUNT];
    uint16_t uBootSig;
} MBR;

#pragma pack(pop)

#endif // MBR_H_INCLUDED
