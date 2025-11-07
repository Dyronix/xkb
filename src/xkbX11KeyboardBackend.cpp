#include "xkbX11KeyboardBackend.h"
#include "xkbWindow.h"

#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include <iostream>

namespace xkb
{
    X11KeyboardBackend::X11KeyboardBackend(Display* display, Window& window)
        : m_Display(display)
        , m_Window(window)
    {}

    X11KeyboardBackend::~X11KeyboardBackend()
    {
        Stop(); 
    }

    bool X11KeyboardBackend::Init()
    {
        if (!m_Display || !m_Window)
        {
            return false;
        }

        return SelectMasks();
    }

    void X11KeyboardBackend::Start()
    {
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
        if (!m_Display || !m_Window)
        {
            return false;
        }

        Bool supported_out = False;
        XkbSetDetectableAutoRepeat(m_Display, True, &supported_out);

        XWindowAttributes xwin_attrs{};

        if (XGetWindowAttributes(m_Display, m_Window, &xwin_attrs))
        {
            XSelectInput(m_Display, m_Window, xwin_attrs.your_event_mask | KeyPressMask | KeyReleaseMask | FocusChangeMask | KeymapStateMask);
        }
        else
        {
            XSelectInput(m_Display, m_Window, KeyPressMask | KeyReleaseMask | FocusChangeMask | KeymapStateMask);
        }

        XFlush(m_Display);

        return true;
    }

    void X11KeyboardBackend::SyncFromServer()
    {
        if (!m_Display)
        {
            return;
        }

        if (!m_SetKey)
        {
            return;
        }

        char keys_state[32]{};

        if (XQueryKeymap(m_Display, keys_state) == 0)
        {
            return;
        }

        for (int key_code = 0; key_code < 256; ++key_code)
        {
            const bool is_down = (keys_state[key_code >> 3] & (1 << (key_code & 7))) != 0;

            KeySym key_sym = XkbKeycodeToKeysym(m_Display, static_cast<unsigned int>(key_code), 0, 0);

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
        if (!m_Display || !m_Window)
        {
            return;
        }

        if (!m_SetKey)
        {
            return;
        }

        m_LastPressTimeStamp.fill(0);

        while (!m_StopThread)
        {
            XEvent ev;
            XNextEvent(m_Display, &ev); // block for at least one event

            for (;;)
            {
                int saved_keycode = 0;
                if (ev.type == KeyPress || ev.type == KeyRelease)
                {
                    saved_keycode = ev.xkey.keycode;
                }

                const Bool filtered = XFilterEvent(&ev, None);
                (void)filtered; // suppress unused warning; filtering helps avoid IME duplicates

                switch (ev.type)
                {
                case KeyPress:
                {
                    const int key_code = saved_keycode;

                    if (key_code >= 0 && key_code < 256)
                    {
                        const unsigned long time_stamp = ev.xkey.time;
                        const unsigned long prev_time_stamp = m_LastPressTimeStamp[key_code];
                        const unsigned long diff = time_stamp - prev_time_stamp; // wrap-safe

                        if (diff == time_stamp || (diff > 0 && diff < (1ul << 31)))
                        {
                            KeySym key_sym = XkbKeycodeToKeysym(m_Display, key_code, 0, 0);
                            if (key_sym != NoSymbol)
                            {       
                                if (auto index = XKeySymToIndex(key_sym))
                                {
                                    m_SetKey(*index, true);
                                }
                            }
                            
                            m_LastPressTimeStamp[key_code] = time_stamp;
                        }
                    }
                }

                break;

                case KeyRelease:
                {
                    const int key_code = saved_keycode;

                    if (key_code >= 0 && key_code < 256)
                    {
                        KeySym key_sym = XkbKeycodeToKeysym(m_Display, key_code, 0, 0);

                        if (key_sym != NoSymbol)
                        {
                            if (auto index = XKeySymToIndex(key_sym))
                            {
                                m_SetKey(*index, false);
                            }
                        }
                    }
                }

                break;

                case FocusOut:

                    // lose focus → pessimistically clear via snapshot on regain
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

                if (XPending(m_Display) == 0)
                {
                    break;
                }

                XNextEvent(m_Display, &ev);
            }
        }
    }
} // namespace xkb