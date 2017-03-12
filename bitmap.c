#include <stdlib.h>
#include <windows.h>
#include "general.h"
#include "debug.h"

static unsigned MxfsCountWordBits(uint32_t v)
{
    v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
    return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
}

unsigned MxfsCountBitsSet(MX_BITMAP *Bm)
{
    unsigned i, Result = 0;
    for(i = 0; i < Bm->Size; ++i)
        Result += MxfsCountWordBits(Bm->Buffer[i]);
    return Result;
}

void MxfsInitBitmap(MX_BITMAP *Bm, uint32_t *Buffer, unsigned BitsCount)
{
    ASSERT(MxfsCountWordBits(0) == 0);
    ASSERT(MxfsCountWordBits(0x11111111) == 8);
    ASSERT(MxfsCountWordBits(0xFFFFFFFF) == 32);
    
    Bm->Buffer = Buffer;
    Bm->Size = BitsCount / 32;
    Bm->Hint = 0;
}

void MxfsDestroyBitmap(MX_BITMAP *Bm)
{
    MxfsFree(Bm->Buffer);
    Bm->Buffer = NULL;
}

int MxfsFindClearBit(MX_BITMAP *Bm)
{
    unsigned i, j;
    
    for(i = 0; i < Bm->Size; ++i)
    {
        if(Bm->Buffer[i] != 0xFFFFFFFF)
            break;
    }
    
    if(i == Bm->Size)
    {
        ERR("Bitmap is full!\n");
        return -ERROR_DISK_FULL;
    }
    
    for(j = 0; j < 32; ++j)
    {
        unsigned Bit = Bm->Buffer[i] & (1 << j);
        if(!Bit) break;
    }
    
    ASSERT(j < 32);
    return i * 32 + j;
}

int MxfsSetBit(MX_BITMAP *Bm, unsigned Bit)
{
    unsigned i = Bit / 32;
    Bit = Bit & 31;
    
    if(i >= Bm->Size)
        return -ERROR_INVALID_PARAMETER;
    
    Bm->Buffer[i] |= (1 << Bit);
    return 0;
}

int MxfsClearBit(MX_BITMAP *Bm, unsigned Bit)
{
    unsigned i = Bit / 32;
    Bit = Bit & 31;
    
    if(i >= Bm->Size)
        return -ERROR_INVALID_PARAMETER;
    
    Bm->Buffer[i] &= ~(1 << Bit);
    return 0;
}

int MxfsGetBit(MX_BITMAP *Bm, unsigned Bit)
{
    unsigned i = Bit / 32;
    Bit = Bit & 31;
    
    if(i >= Bm->Size)
        return -ERROR_INVALID_PARAMETER;
    
    if(Bm->Buffer[i] & (1 << Bit))
        return 1;
    else
        return 0;
}
