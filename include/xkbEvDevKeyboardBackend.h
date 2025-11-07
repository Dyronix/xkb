#pragma once

#include "xkbKeyboardBackend.h"

#include <string>

namespace xkb
{
    class EvdevKeyboardBackend : public IKeyboardBackend
    {
    public:
        ~EvdevKeyboardBackend() override;

        bool Init() override;
        void Start() override;
        void Stop() override;

    private:
        void Loop();

        int m_DeviceFileDirectory = -1;
        std::thread m_Thread;
        bool m_StopThread{false};
    };
} // namespace xkb