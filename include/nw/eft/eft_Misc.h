#ifndef EFT_MISC_H_
#define EFT_MISC_H_

#include <nw/eft/eft_Heap.h>

namespace nw { namespace eft {

void SetStaticHeap(Heap* staticHeap);
void* AllocFromStaticHeap(u32 size, u32 alignment = 128);
void FreeFromStaticHeap(void* ptr);
u32 GetAllocedSizeFromStaticHeap();

void SetDynamicHeap(Heap* dynamicHeap);
void* AllocFromDynamicHeap(u32 size, u32 alignment = 128);
void FreeFromDynamicHeap(void* ptr, bool immediate = true);

void InitializeDelayFreeList(u32 freeListNum);
void FinalizeDelayFreeList();
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

typedef void (*OutputMessageCallback)(const char* format, va_list vargs);

void SetOutputLogCallBack(OutputMessageCallback cb);
void SetOutputWarningCallBack(OutputMessageCallback cb);
void SetOutputErrorCallBack(OutputMessageCallback cb);

} } // namespace nw::eft

#endif // EFT_MISC_H_
