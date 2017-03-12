#ifndef PARTITIONS_H_INCLUDED
#define PARTITIONS_H_INCLUDED

#include <stdio.h>
#include "mbr.h"
#include "disk_image.h"

typedef struct
{
    unsigned PartIdx;
    unsigned SubpartIdx;
    unsigned Offset;
    unsigned Size;
} SUBPARTITION;

typedef struct
{
    SUBPARTITION *Entries;
    unsigned Count;
} SUBPART_VECTOR;

int LoadMBR(DISK_IMG *pImg, MBR *pMbr);
int FindSubpartitions(DISK_IMG *pImg, SUBPART_VECTOR *pVect);

#endif // PARTITIONS_H_INCLUDED
