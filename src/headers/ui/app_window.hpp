#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "common/types.hpp"
#include "input/input_mapping.hpp"

namespace nes {

class InputSettingsMenu;

struct OverlayState {
    bool paused {false};
    bool gamepad_connected {false};
    bool in_vblank {false};
    bool sprite_zero_hit {false};
    double fps {0.0};
    std::uint64_t frame_counter {0};
    u16 pc {0};
    u8 controller_state {0};
    std::string rom_name {};
};

class AppWindow {
public:
    AppWindow();
    ~AppWindow();

    AppWindow(const AppWindow&) = delete;
    AppWindow& operator=(const AppWindow&) = delete;

    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] bool has_gamepad() const noexcept;
    [[nodiscard]] bool settings_open() const noexcept;
    [[nodiscard]] bool process_events();

    [[nodiscard]] u8 poll_controller_state() const;
    [[nodiscard]] bool consume_pause_pressed() noexcept;
    [[nodiscard]] bool consume_reset_pressed() noexcept;
    [[nodiscard]] bool consume_step_frame_pressed() noexcept;
    [[nodiscard]] bool consume_frame_counter_toggle_pressed() noexcept;

    [[nodiscard]] const InputMapping& input_mapping() const noexcept;
    void set_input_mapping(const InputMapping& mapping) noexcept;
    void reset_input_mapping_to_defaults() noexcept;

    void set_title(const std::string& title);
    void present_frame(
        const std::array<u8, 256U * 240U>& frame,
        const OverlayState& overlay
    );

private:
    void try_open_first_gamepad();
    void close_gamepad() noexcept;

    void* window_ {nullptr};
    void* renderer_ {nullptr};
    void* texture_ {nullptr};
    void* gamepad_ {nullptr};

    std::unique_ptr<InputSettingsMenu> input_settings_menu_ {};
    InputMapping input_mapping_ {};

    bool available_ {false};
    bool pause_pressed_ {false};
    bool reset_pressed_ {false};
    bool step_frame_pressed_ {false};
    bool frame_counter_toggle_pressed_ {false};
};

} // namespace nes