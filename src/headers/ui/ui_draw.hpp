#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nes::ui {

std::string uppercase_text(std::string text);
std::string limit_text(std::string text, std::size_t max_len);

void draw_rect(
    std::vector<std::uint32_t>& pixels,
    int x,
    int y,
    int w,
    int h,
    std::uint32_t color
);

void draw_text(
    std::vector<std::uint32_t>& pixels,
    int x,
    int y,
    const std::string& text,
    std::uint32_t color
);

} // namespace nes::ui