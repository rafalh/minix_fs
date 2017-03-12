#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <stdio.h>
#include <assert.h>

#ifdef NDEBUG

#define ERR(...) printf(__VA_ARGS__)
#define WARN(...) 
#define INFO(...) 
#define TRACE(fmt, ...) 
#define UNIMPLEMENTED(fmt, ...) 
#define ASSERT(...) 
#define MxfsDump(...) 

#else // NDEBUG

#define ERR(...) printf(__VA_ARGS__)
#define WARN(...) printf(__VA_ARGS__)
#define INFO(...) //printf(__VA_ARGS__)
#define TRACE(fmt, ...) //printf("Trace: %s(" fmt ")\n", __func__, ##__VA_ARGS__)
#define UNIMPLEMENTED(fmt, ...) printf("Unimplemented function %s(" fmt ") called\n", __func__, ##__VA_ARGS__)
#define ASSERT(...) assert(__VA_ARGS__)

void MxfsDump(const void *pvBuf, unsigned uSize);

#endif // NDEBUG

#endif // DEBUG_H_INCLUDED
