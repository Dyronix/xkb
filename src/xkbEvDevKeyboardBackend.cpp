#include "xkbEvDevKeyboardBackend.h"
#include "xkbInputDetection.h"

#include <linux/input.h>

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>

namespace xkb
{
    EvdevKeyboardBackend::~EvdevKeyboardBackend()
    {
        Stop(); 
    }

    bool EvdevKeyboardBackend::Init()
    {
        auto node = FindActiveKeyboardEventNode();
        if (!node)
        {
            std::cerr << "[Evdev] No keyboard event node found\n";
            return false;
        }

        const std::string path = "/dev/input/" + *node;
        m_DeviceFileDirectory = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (m_DeviceFileDirectory < 0)
        {
            std::cerr << "[Evdev] open(" << path << ") failed: " << std::strerror(errno) << "\n";
            return false;
        }

        return true;
    }

    void EvdevKeyboardBackend::Start()
    {
        m_Thread = std::thread(&EvdevKeyboardBackend::Loop, this);
    }

    void EvdevKeyboardBackend::Stop()
    {
        if (m_DeviceFileDirectory >= 0)
        {
            ::close(m_DeviceFileDirectory);
            m_DeviceFileDirectory = -1;
        }
        
        m_StopThread = true;
        if (m_Thread.joinable())
        {
            m_Thread.join();
        }
    }

    void EvdevKeyboardBackend::Loop()
    {
        if (m_DeviceFileDirectory < 0 || !m_SetKey)
        {
            return;
        }

        input_event ev{};
        while (!m_StopThread)
        {
            const ssize_t n = ::read(m_DeviceFileDirectory, &ev, sizeof(ev));
            if (n == sizeof(ev))
            {
                if (ev.type == EV_KEY)
                {
                    const bool down = (ev.value != 0);
                    if (ev.code < 256)
                    {
                        m_SetKey(static_cast<size_t>(ev.code), down);
                    }
                }
            }
            else if (n == -1)
            {
                if (errno == EINTR)
                {
                    continue; // retry
                }
                break;        // m_DeviceFileDirectory likely closed in Stop()
            }
            else
            {
                break; // EOF/partial
            }
        }
    }
} // namespace xkb