#include "xkbInputDetection.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace 
{
    static std::optional<unsigned int> ParseHexToUint(const std::string &s)
    {
        std::stringstream ss(s);

        unsigned int v = 0;
        ss >> std::hex >> v;
        if (ss.fail())
        {
            return std::nullopt;
        }

        return v;
    }
}

namespace xkb
{
    std::optional<std::string> FindActiveKeyboardEventNode()
    {
        std::ifstream f("/proc/bus/input/devices");
        if (!f.is_open())
        {
            std::cerr << "[InputDiscovery] Cannot open devices file\n";
            return std::nullopt;
        }

        std::string line, last_handlers;

        while (std::getline(f, line))
        {
            if (line.rfind("H: Handlers=", 0) == 0)
            {
                last_handlers = line;
            }
            else if (line.rfind("B: EV=", 0) == 0)
            {
                const std::string hex = line.substr(6);
                auto val = ParseHexToUint(hex);
                if (!val)
                {
                    continue;
                }

                constexpr unsigned int keyboard_mask = 0x120013; // same mask as before
                if ((*val & keyboard_mask) == keyboard_mask)
                {
                    std::istringstream hs(last_handlers);
                    std::string token, last_token;

                    while (hs >> token)
                    {
                        last_token = token;
                    }

                    if (last_token.rfind("event", 0) == 0)
                    {
                        return last_token; // e.g., "event0"
                    }
                }
            }
        }
        
        return std::nullopt;
    }
}