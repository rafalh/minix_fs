#include "debug.h"
#include <ctype.h>

#ifndef NDEBUG
void MxfsDump(const void *pvBuf, unsigned uSize)
{
    const unsigned COLS = 16;
    unsigned i = 0, j;
    unsigned char *pcBuf = (unsigned char*)pvBuf;
    
    while(i < uSize)
    {
        for(j = 0; j < COLS; ++j)
        {
            if(i + j < uSize)
                printf("%02X ", pcBuf[i + j]);
            else
                printf("   ");
        }
        
        printf(" ");
        
        for(j = 0; j < COLS; ++j)
        {
            if(i + j < uSize)
                printf("%c", isprint(pcBuf[i + j]) ? pcBuf[i + j] : '.');
            else
                printf(" ");
        }
        
        printf("\n");
        i += COLS;
    }
}
#endif // NDEBUG
