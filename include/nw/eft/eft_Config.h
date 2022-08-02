#ifndef EFT_CONFIG_H_
#define EFT_CONFIG_H_

#include <nw/eft/eft_Heap.h>

namespace nw { namespace eft {

class Config
{
public:
    enum ConfigDefault
    {
        EFT_DEFAULT_EMITTER_NUM                     = 64,
        EFT_DEFAULT_PARTICLE_NUM                    = 1024,
        EFT_DEFAULT_EMITTER_SET_NUM                 = 32,
        EFT_DEFAULT_RESOURCE_NUM                    = 8,
        EFT_DEFAULT_RESOURCE_VIEWER_RESERVE_NUM     = 32,
        EFT_DEFAULT_STRIPE_NUM                      = 32,
        EFT_DEFAULT_DOUBLE_BUFFER_SIZE              = 128 * 1024,
        EFT_DEFAULT_FORCE_4BYTE = 0x80000000
    };

    Config() : mHeap                    ( NULL )
             , mDynamicHeap             ( NULL )
             , mEmitterNum              ( EFT_DEFAULT_EMITTER_NUM )
             , mParticleNum             ( EFT_DEFAULT_PARTICLE_NUM )
             , mEmitterSetNum           ( EFT_DEFAULT_EMITTER_SET_NUM )
             , mResourceNum             ( EFT_DEFAULT_RESOURCE_NUM )
             , mStripeNum               ( EFT_DEFAULT_STRIPE_NUM )
             , mSizeDoubleBufferSize    ( EFT_DEFAULT_DOUBLE_BUFFER_SIZE )
             , mIsEnableTripleBuffer    ( false )
             , mViewerResourceNum       ( EFT_DEFAULT_RESOURCE_VIEWER_RESERVE_NUM )
#if defined( NW_DEBUG )
             , mSuppressLog             ( false )
#else
             , mSuppressLog             ( true )
#endif
    {
    }

    virtual ~Config()
    {
    }

    void SetEffectHeap(Heap* heap)
    {
        mHeap = heap;
    }

    Heap* GetEffectHeap() const
    {
        return mHeap;
    }

    void SetEffectDynamicHeap(Heap* heap)
    {
        mDynamicHeap = heap;
    }

    Heap* GetEffectDynamicHeap() const
    {
        if (mDynamicHeap)
            return mDynamicHeap;

        return mHeap;
    }

    void SetEmitterNum(u32 num)
    {
        mEmitterNum = num;
    }

    u32 GetEmitterNum() const
    {
        return mEmitterNum;
    }

    void SetParticleNum(u32 num)
    {
        mParticleNum = num;
    }

    u32 GetParticleNum() const
    {
        return mParticleNum;
    }

    void SetEmitterSetNum(u32 num)
    {
        mEmitterSetNum = num;
    }

    u32 GetEmitterSetNum() const
    {
        return mEmitterSetNum;
    }

    void SetResourceNum(u32 num)
    {
        mResourceNum = num;
    }

    u32 GetResourceNum() const
    {
        return mResourceNum;
    }

    void SetStripeNum(u32 num)
    {
        mStripeNum = num;
    }

    u32 GetStripeNum() const
    {
        return mStripeNum;
    }

    void SetDoubleBufferSize(u32 size)
    {
        mSizeDoubleBufferSize = size;
    }

    u32 GetDoubleBufferSize() const
    {
        return mSizeDoubleBufferSize;
    }

    void SetSuppressionLog(bool flag)
    {
        mSuppressLog = flag;
    }

    bool IsSuppressionLog() const
    {
        return mSuppressLog;
    }

    void SetEnableTripleBuffer(bool isUseTripeBuffer)
    {
        mIsEnableTripleBuffer = isUseTripeBuffer;
    }

    bool IsEnableTripleBuffer() const
    {
        return mIsEnableTripleBuffer;
    }

    u32 GetViewerResourceNum() const
    {
        return mViewerResourceNum;
    }

protected:
    Heap*           mHeap;
    Heap*           mDynamicHeap;
    u32             mEmitterNum;
    u32             mParticleNum;
    u32             mEmitterSetNum;
    u32             mStripeNum;
    u32             mResourceNum;
    u32             mSizeDoubleBufferSize;
    bool            mSuppressLog;
    bool            mIsEnableTripleBuffer;
    u32             mViewerResourceNum;
};
static_assert(sizeof(Config) == 0x2C, "nw::eft::Config size mismatch");

} } // namespace nw::eft

#endif // EFT_CONFIG_H_
