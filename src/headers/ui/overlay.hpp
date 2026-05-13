#pragma once

#include <cstdint>
#include <vector>

#include "ui/app_window.hpp"

namespace nes::ui {

void draw_overlay(
    std::vector<std::uint32_t>& pixels,
    const OverlayState& overlay
);

} // namespace nes::ui