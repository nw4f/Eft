#ifndef EFT_MISC_H_
#define EFT_MISC_H_

#include <nw/typeDef.h>

namespace nw { namespace eft {

class Heap;

void SetStaticHeap(Heap* heap);
void* AllocFromStaticHeap(u32 size, u32 alignment = 0x80);
u32 GetAllocedSizeFromStaticHeap();

void SetDynamicHeap(Heap* heap);
void* AllocFromDynamicHeap(u32 size, u32 alignment = 0x80);
void FreeFromDynamicHeap(void* ptr, bool noDelay);

void InitializeDelayFreeList(u32 max);
void FlushDelayFreeList();
void AddFreeListForDynamicHeap(void* ptr);

void SetSuppressOutputLog(bool suppressLog);

// This function is just for printing debug messages
void OutputLog(const char* fmt, ...);
#define LOG OutputLog

// This function is just for printing warning messages
void OutputWarning(const char* fmt, ...);
#define WARNING OutputWarning

// This function is supposed to print the error message and terminate the program
void OutputError(const char* fmt, ...);
#define ERROR OutputError

} } // namespace nw::eft

#endif // EFT_MISC_H_
