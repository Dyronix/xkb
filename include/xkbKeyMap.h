#pragma once

#include <optional>
#include <cstddef>

namespace xkb
{
    using KeySym_Fwd = unsigned long;

    std::optional<size_t> XKeySymToIndex(KeySym_Fwd sym);
}