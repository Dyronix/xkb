#pragma once

#include <optional>
#include <string>

namespace xkb
{
    std::optional<std::string> FindActiveKeyboardEventNode();
} // namespace xkb