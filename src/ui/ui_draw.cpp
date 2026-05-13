#include "ui/ui_draw.hpp"

#include <array>
#include <cctype>

namespace nes::ui {
namespace {
// Draws a filled rectangle on the pixel buffer. The rectangle is defined by its top-left corner (x, y) and its width (w) and height (h).
// The color parameter is a 32-bit ARGB value that specifies the color of the rectangle.

/// @brief  Draws a filled rectangle on the pixel buffer. The rectangle is defined by its top-left corner (x, y) and its width (w) and height (h).
/// @param pixels The pixel buffer to draw on. This is a vector of 32-bit ARGB values representing the screen pixels.
/// @param x The x-coordinate of the top-left corner of the rectangle.
/// @param y The y-coordinate of the top-left corner of the rectangle.
/// @param w The width of the rectangle.
/// @param h The height of the rectangle.
/// @param ch The color of the rectangle, represented as a 32-bit ARGB value.
/// @return None. This function modifies the pixels vector in place.

std::array<std::uint8_t, 7> glyph_for(char ch) {
    switch (ch) {
    case '0': return {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    case '1': return {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    case '3': return {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    case '5': return {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
    case '6': return {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    case '7': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    case '8': return {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    case '9': return {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E};

    case 'A': return {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    case 'B': return {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    case 'C': return {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    case 'D': return {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C};
    case 'E': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    case 'F': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    case 'G': return {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
    case 'H': return {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    case 'I': return {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
    case 'J': return {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E};
    case 'K': return {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    case 'L': return {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    case 'M': return {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    case 'N': return {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    case 'Q': return {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
    case 'R': return {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    case 'T': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    case 'U': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    case 'V': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
    case 'W': return {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
    case 'X': return {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
    case 'Y': return {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    case 'Z': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};

    case ':': return {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00};
    case '.': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
    case '-': return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    case '/': return {0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10};
    case '_': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
    case '$': return {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04};
    case ' ': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    default:  return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    }
}

void draw_char(
    std::vector<std::uint32_t>& pixels,
    int x,
    int y,
    char ch,
    std::uint32_t color
) {
    const auto glyph = glyph_for(ch);

    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if ((glyph[static_cast<std::size_t>(row)] & (1U << (4 - col))) == 0U) {
                continue;
            }

            const int px = x + col;
            const int py = y + row;
            if (px < 0 || px >= 256 || py < 0 || py >= 240) {
                continue;
            }

            pixels[static_cast<std::size_t>(py) * 256U + static_cast<std::size_t>(px)] =
                color;
        }
    }
}

} // namespace

std::string uppercase_text(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));

        const bool ok =
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == ' ' ||
            ch == ':' ||
            ch == '.' ||
            ch == '-' ||
            ch == '/' ||
            ch == '_' ||
            ch == '$';

        if (!ok) {
            ch = '-';
        }
    }

    return text;
}

std::string limit_text(std::string text, std::size_t max_len) {
    if (text.size() <= max_len) {
        return text;
    }

    if (max_len <= 3U) {
        return text.substr(0, max_len);
    }

    return text.substr(0, max_len - 3U) + "...";
}

void draw_rect(
    std::vector<std::uint32_t>& pixels,
    int x,
    int y,
    int w,
    int h,
    std::uint32_t color
) {
    for (int row = 0; row < h; ++row) {
        const int py = y + row;
        if (py < 0 || py >= 240) {
            continue;
        }

        for (int col = 0; col < w; ++col) {
            const int px = x + col;
            if (px < 0 || px >= 256) {
                continue;
            }

            pixels[static_cast<std::size_t>(py) * 256U + static_cast<std::size_t>(px)] =
                color;
        }
    }
}

void draw_text(
    std::vector<std::uint32_t>& pixels,
    int x,
    int y,
    const std::string& text,
    std::uint32_t color
) {
    int cursor_x = x;

    for (const char ch : text) {
        draw_char(pixels, cursor_x, y, ch, color);
        cursor_x += 6;
    }
}

} // namespace nes::ui