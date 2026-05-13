#pragma once

#include <array>
#include <cstdint>

#include "common/types.hpp"

namespace nes {

[[nodiscard]] std::uint32_t nes_color_to_argb(u8 color_index) noexcept;

} // namespace nes