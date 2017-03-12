#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

#include <stdint.h>

typedef struct
{
    uint32_t *Buffer;
    unsigned Size;
    unsigned Hint;
} MX_BITMAP;

void MxfsInitBitmap(MX_BITMAP *Bm, uint32_t *Buffer, unsigned BitsCount);
void MxfsDestroyBitmap(MX_BITMAP *Bm);
unsigned MxfsCountBitsSet(MX_BITMAP *Bm);
int MxfsFindClearBit(MX_BITMAP *Bm);
int MxfsSetBit(MX_BITMAP *Bm, unsigned Bit);
int MxfsClearBit(MX_BITMAP *Bm, unsigned Bit);
int MxfsGetBit(MX_BITMAP *Bm, unsigned Bit);

#endif // BITMAP_H_INCLUDED
