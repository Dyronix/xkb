#pragma once

#include "xkbKeyboardBackend.h"

#include <X11/Xlib.h>

#include <array>

namespace xkb
{
    class X11KeyboardBackend : public IKeyboardBackend
    {
    public:
        X11KeyboardBackend(Display* display, Window& window);
        ~X11KeyboardBackend() override;

        bool Init() override;
        void Start() override;
        void Stop() override;

    private:
        void Loop();
        bool SelectMasks();
        void SyncFromServer();

        Display* m_AppDisplay = nullptr;
        Display* m_KeyboardDisplay = nullptr;
        Window m_Window = 0;

        std::thread m_Thread;
        std::array<unsigned long, 256> m_LastPressTimeStamp{};
        bool m_StopThread{false};
        bool m_MasksSelected{false};
    };
} // namespace xkb