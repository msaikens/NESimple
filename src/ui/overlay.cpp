#include "ui/overlay.hpp"

#include "ui/ui_draw.hpp"

#include <iomanip>
#include <sstream>

namespace nes::ui {

void draw_overlay(
    std::vector<std::uint32_t>& pixels,
    const OverlayState& overlay
) {
    constexpr std::uint32_t kPanel = 0xCC101820U;
    constexpr std::uint32_t kBorder = 0xFFB8B8B8U;
    constexpr std::uint32_t kText = 0xFFFFFEFFU;
    constexpr std::uint32_t kPausedText = 0xFFFFFF66U;

    draw_rect(pixels, 0, 0, 256, 19, kPanel);
    draw_rect(pixels, 0, 0, 256, 1, kBorder);
    draw_rect(pixels, 0, 18, 256, 1, kBorder);

    draw_rect(pixels, 0, 229, 256, 11, kPanel);
    draw_rect(pixels, 0, 229, 256, 1, kBorder);

    const std::string rom_name = limit_text(
        uppercase_text(overlay.rom_name.empty() ? "NO ROM" : overlay.rom_name),
        24U
    );

    const std::string run_state = overlay.paused ? "PAUSED" : "RUNNING";
    const std::uint32_t state_color = overlay.paused ? kPausedText : kText;

    std::ostringstream fps_line;
    fps_line << "FPS " << std::fixed << std::setprecision(1) << overlay.fps;

    if (overlay.frame_counter != 0U) {
        fps_line << "  FRAME " << overlay.frame_counter;
    }

    draw_text(pixels, 2, 2, "ROM " + rom_name, kText);
    draw_text(pixels, 2, 11, run_state, state_color);
    draw_text(pixels, 50, 11, uppercase_text(fps_line.str()), kText);

    draw_text(pixels, 2, 231, "P PAUSE N STEP R RESET F2 INPUT F3 FRAME", kText);
}

} // namespace nes::ui