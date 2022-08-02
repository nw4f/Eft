#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Misc.h>

#include <cstring>

namespace nw { namespace eft {

u32 g_StaticHeapAllocedSize = 0;
Heap* g_StaticHeap = NULL;
Heap* g_DynamicHeap = NULL;

bool g_SuppressLog = false;

#define  _DELAY_FREE_TIME   (3)

void** g_DelayFreeList[_DELAY_FREE_TIME];
u32 g_DelayFreeListNum[_DELAY_FREE_TIME];
u32 g_DelayFreeListMax = 0;
u32 g_DelayFreeListCntr = 0;

void OutputError(const char* fmt, ...)
{
    // This function is supposed to print the error message and terminate the program
}

void SetStaticHeap(Heap* staticHeap)
{
    g_StaticHeap = staticHeap;
}

void* AllocFromStaticHeap(u32 size, u32 align)
{
    void* ptr = g_StaticHeap->Alloc(size, align);
    g_StaticHeapAllocedSize += size;
    return ptr;
}

void FreeFromStaticHeap(void* ptr)
{
    return g_StaticHeap->Free( ptr );
}

void SetDynamicHeap(Heap* dynamicHeap)
{
    g_DynamicHeap = dynamicHeap;
}

void OutputWarning(const char* fmt, ...)
{
    // This function is just for printing warning messages
}

void* AllocFromDynamicHeap(u32 size, u32 align)
{
    // u32 allocSize = nw::ut::RoundUp(size, 0x100);
    u32 allocSize = size + (0x100 - 1) & ~(0x100 - 1);
    void* ptr = g_DynamicHeap->Alloc(allocSize, align);
    return ptr;
}

void AddFreeListForDynamicHeap(void* ptr)
{
    g_DelayFreeList[g_DelayFreeListCntr][g_DelayFreeListNum[g_DelayFreeListCntr]] = ptr;
    g_DelayFreeListNum[g_DelayFreeListCntr]++;
}

void FreeFromDynamicHeap(void* ptr, bool immediate)
{
    if (immediate)
        return g_DynamicHeap->Free(ptr);
    else
        AddFreeListForDynamicHeap(ptr);
}

void InitializeDelayFreeList(u32 freeListNum)
{
    g_DelayFreeListMax = freeListNum;

    for (u32 i = 0; i < _DELAY_FREE_TIME; i++)
    {
        g_DelayFreeList[i] = static_cast<void**>(AllocFromStaticHeap(sizeof(void*) * g_DelayFreeListMax));
        memset(g_DelayFreeList[i], 0, sizeof(void*) * g_DelayFreeListMax);
        g_DelayFreeListNum[i] = 0;
    }
}

void FinalizeDelayFreeList()
{
    for (u32 i = 0; i < _DELAY_FREE_TIME; i++)
    {
        u32 j = 0;

        while (g_DelayFreeList[i][j])
        {
            g_DynamicHeap->Free(g_DelayFreeList[i][j]);
            j++;
        }

        g_StaticHeap->Free(g_DelayFreeList[i]);
        g_DelayFreeList[i] = NULL;
    }
}

void FlushDelayFreeList()
{
    if (++g_DelayFreeListCntr == _DELAY_FREE_TIME)
        g_DelayFreeListCntr = 0;

    s32 i = 0;
    while (g_DelayFreeList[g_DelayFreeListCntr][i])
    {
        g_DynamicHeap->Free(g_DelayFreeList[g_DelayFreeListCntr][i]);
        i++;
    }

    memset(g_DelayFreeList[g_DelayFreeListCntr], 0, sizeof(void*) * i);
    g_DelayFreeListNum[g_DelayFreeListCntr] = 0;
}

void SetSuppressOutputLog(bool flag)
{
    g_SuppressLog = flag;
}

void OutputLog(const char* fmt, ...)
{
    // This function is just for printing debug messages
}

u32 GetAllocedSizeFromStaticHeap()
{
    return g_StaticHeapAllocedSize;
}

} } // namespace nw::eft
