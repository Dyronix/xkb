#pragma once

#include <functional>
#include <atomic>
#include <thread>

namespace xkb
{
    class IKeyboardBackend
    {
    public:
        using SetKeyFn = std::function<void(size_t /*engineKeyIndex*/, bool /*down*/)>;

        virtual ~IKeyboardBackend() = default;
        virtual bool Init() = 0;  
        virtual void Start() = 0; 
        virtual void Stop() = 0;  

        void SetCallbacks(SetKeyFn setKey) { m_SetKey = std::move(setKey); }

    protected:
        SetKeyFn m_SetKey;
    };
} // namespace xkb