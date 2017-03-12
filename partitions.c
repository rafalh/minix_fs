#include <assert.h>
#include <stdlib.h>
#include "partitions.h"
#include "debug.h"

int LoadMBR(DISK_IMG *pImg, MBR *pMbr)
{
    assert(sizeof(*pMbr) == 512);
    
    int err = ImgRead(pImg, 0, sizeof(*pMbr), pMbr);
    if(err < 0)
    {
        ERR("Failed to load MBR\n");
        return err;
    }
    
    if(pMbr->uBootSig != MBR_BOOT_SIG)
    {
        WARN("Invalid signature: %u\n", pMbr->uBootSig);
        return -1;
    }
    
    return 0;
}

void AddSubpartToVect(SUBPART_VECTOR *pVect, unsigned Offset, unsigned Size, unsigned PartIdx, unsigned SubpartIdx)
{
    if(pVect->Count % 4 == 0)
        pVect->Entries = realloc(pVect->Entries, (pVect->Count + 4) * sizeof(SUBPARTITION));
    
    SUBPARTITION *pSubpart = &pVect->Entries[pVect->Count++];
    pSubpart->Offset = Offset;
    pSubpart->Size = Size;
    pSubpart->PartIdx = PartIdx;
    pSubpart->SubpartIdx = SubpartIdx;
}

int FindSubpartsOnPart(DISK_IMG *pImg, unsigned Offset, unsigned Size, unsigned PartIdx, SUBPART_VECTOR *pVect)
{
    MBR Ebr;
    
    if(Size > 512)
    {
        int err = ImgRead(pImg, Offset, sizeof(Ebr), &Ebr);
        if(err < 0)
            return err;
    } else
        Ebr.uBootSig = 0;
    
    if(Ebr.uBootSig != MBR_BOOT_SIG)
    {
        // No subpartitions
        AddSubpartToVect(pVect, Offset, Size, PartIdx, 0);
        return 0;
    }
    
    unsigned i;
    for(i = 0; i < 4; ++i)
    {
        MBR_PARTITION *pPart = &Ebr.Partitions[i];
        if(pPart->Type == MBR_PART_FREE) continue;
        
        unsigned SubpartOffset = pPart->uFirstSectLba * 512;
        unsigned SubpartSize = pPart->cSect * 512;
        AddSubpartToVect(pVect, SubpartOffset, SubpartSize, PartIdx, i);
        
        TRACE("EBR[%u]: Type 0x%x uFirstSectLba 0x%x cSect 0x%x off 0x%x\n", i, pPart->Type, pPart->uFirstSectLba, pPart->cSect, pPart->uFirstSectLba*512);
    }
    
    return 0;
}

int FindSubpartitions(DISK_IMG *pImg, SUBPART_VECTOR *pVect)
{
    pVect->Count = 0;
    pVect->Entries = NULL;
    
    MBR Mbr;
    int iRet = LoadMBR(pImg, &Mbr);
    if(iRet < 0)
    {
        // No partitions
        AddSubpartToVect(pVect, 0, ImgGetSize(pImg), 0, 0);
        return 0;
    }
    
    unsigned i;
    for(i = 0; i < MBR_PART_COUNT; ++i)
    {
        MBR_PARTITION *pPart = &Mbr.Partitions[i];
        if(pPart->Type == MBR_PART_MINIX14)
        {
            TRACE("Found MINIX partition: %u. MBR entry, lba 0x%x, size 0x%x\n", i, pPart->uFirstSectLba, pPart->cSect);
            unsigned Offset = pPart->uFirstSectLba * 512;
            unsigned Size = pPart->cSect * 512;
            FindSubpartsOnPart(pImg, Offset, Size, i, pVect);
        }
        else if(pPart->Type != MBR_PART_FREE)
            INFO("Unknown partition %u: type 0x%x, lba 0x%x, size 0x%x\n", i, pPart->Type, pPart->uFirstSectLba, pPart->cSect);
    }
    
    if(pVect->Count == 0)
        ERR("Failed to find a MINIX partition\n");
    
    return 0;
}
