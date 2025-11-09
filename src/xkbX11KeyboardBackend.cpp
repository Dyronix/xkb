#include "xkbX11KeyboardBackend.h"
#include "xkbKeyMap.h"

#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include <iostream>

namespace
{
    // Predicate used by XIfEvent / XCheckIfEvent so we only remove keyboard events
    Bool KeyOnlyPredicate(Display* /*dpy*/, XEvent* ev, XPointer /*arg*/)
    {
        return ev->type == KeyPress
            || ev->type == KeyRelease
            || ev->type == FocusIn
            || ev->type == FocusOut
            || ev->type == KeymapNotify
            || ev->type == MappingNotify;
    }
}

namespace xkb
{
    X11KeyboardBackend::X11KeyboardBackend(Display* display, Window& window)
        : m_AppDisplay(display)
        , m_Window(window)
    {}

    X11KeyboardBackend::~X11KeyboardBackend()
    {
        Stop(); 

        if (m_KeyboardDisplay)
        {
            // Only close the display we own
            XCloseDisplay(m_KeyboardDisplay);
            m_KeyboardDisplay = nullptr;
        }
    }

    bool X11KeyboardBackend::Init()
    {
        if (!m_AppDisplay || !m_Window)
        {
            return false;
        }

        // Open a SECOND connection using the same display string.
        // Each connection has its own event queue; we won't consume the app's events.
        const char* name = XDisplayString(m_AppDisplay);
        m_KeyboardDisplay = XOpenDisplay(name);
        if (!m_KeyboardDisplay)
        {
            return false;
        }

        // Make key repeat detectable on our connection
        Bool supported_out = False;
        XkbSetDetectableAutoRepeat(m_KeyboardDisplay, True, &supported_out);

        m_MasksSelected = SelectMasks();

        return m_MasksSelected;
    }

    void X11KeyboardBackend::Start()
    {
        m_StopThread = false;
        m_Thread = std::thread(&X11KeyboardBackend::Loop, this);
    }

    void X11KeyboardBackend::Stop()
    {
        m_StopThread = true;
        if (m_Thread.joinable())
        {
            m_Thread.join();
        }
    }

    bool X11KeyboardBackend::SelectMasks()
    {
        if (!m_KeyboardDisplay || !m_Window)
        {
            return false;
        }

        long mask = KeyPressMask | KeyReleaseMask | FocusChangeMask | KeymapStateMask;

        XSelectInput(m_KeyboardDisplay, m_Window, mask);
        XFlush(m_KeyboardDisplay);

        return true;
    }

    void X11KeyboardBackend::SyncFromServer()
    {
        if (!m_KeyboardDisplay || !m_SetKey)
        {
            return;
        }

        char keys_state[32]{};
        if (XQueryKeymap(m_KeyboardDisplay, keys_state) == 0)
        {
            return;
        }

        for (int key_code = 0; key_code < 256; ++key_code)
        {
            const bool is_down = (keys_state[key_code >> 3] & (1 << (key_code & 7))) != 0;

            KeySym key_sym = XkbKeycodeToKeysym(m_KeyboardDisplay, static_cast<unsigned int>(key_code), 0, 0);
            if (key_sym != NoSymbol)
            {
                if (auto index = XKeySymToIndex(key_sym))
                {
                    m_SetKey(*index, is_down);
                }
            }
        }
    }

    void X11KeyboardBackend::Loop()
    {
        if (!m_KeyboardDisplay || !m_Window || !m_SetKey || !m_MasksSelected)
        {
            return;
        }

        // Initialize timestamps
        m_LastPressTimeStamp.fill(0);

        // Blocking predicate loop:
        //  - waits for the next keyboard-related event
        //  - NEVER removes non-keyboard events from any queue
        while (!m_StopThread)
        {
            XEvent ev{};
            XIfEvent(m_KeyboardDisplay, &ev, &KeyOnlyPredicate, nullptr);

            // Optional IME filter; do not skip dispatch because of it
            XFilterEvent(&ev, None);

            switch (ev.type)
            {
            case KeyPress:
            {
                const int key_code = ev.xkey.keycode;
                if (key_code >= 0 && key_code < 256)
                {
                    const unsigned long ts = ev.xkey.time;
                    const unsigned long prev = m_LastPressTimeStamp[key_code];
                    const unsigned long diff = ts - prev; // wrap-safe

                    // Same duplicate filter that GLFW uses conceptually
                    if (diff == ts || (diff > 0 && diff < (1ul << 31)))
                    {
                        KeySym sym = XkbKeycodeToKeysym(m_KeyboardDisplay, key_code, 0, 0);
                        if (sym != NoSymbol)
                        {
                            if (auto idx = XKeySymToIndex(sym))
                            {
                                m_SetKey(*idx, true);
                            }
                        }
                        m_LastPressTimeStamp[key_code] = ts;
                    }
                }
            } break;

            case KeyRelease:
            {
                const int key_code = ev.xkey.keycode;
                if (key_code >= 0 && key_code < 256)
                {
                    KeySym sym = XkbKeycodeToKeysym(m_KeyboardDisplay, key_code, 0, 0);
                    if (sym != NoSymbol)
                    {
                        if (auto idx = XKeySymToIndex(sym))
                        {
                            m_SetKey(*idx, false);
                        }
                    }
                }
            } break;

            case FocusOut:
                // pessimistic clear on focus loss
                for (size_t i = 0; i < 256; ++i)
                {
                    m_SetKey(i, false);
                }
                
                m_LastPressTimeStamp.fill(0);
                break;

            case FocusIn:
            case KeymapNotify:
                SyncFromServer();
                break;

            case MappingNotify:
                XRefreshKeyboardMapping(&ev.xmapping);
                break;

            default:
                break;
            }

            // Small yield to avoid burning a core if events flood
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }
} // namespace xkb