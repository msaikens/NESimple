#pragma once

#include <cstdint>
#include <vector>

#include "input/input_mapping.hpp"

#if defined(NESIMPLE_HAS_SDL)
#include <SDL3/SDL.h>
#endif

namespace nes {

class InputSettingsMenu {
public:
    InputSettingsMenu() = default;

    [[nodiscard]] bool open() const noexcept;
    void toggle() noexcept;
    void close() noexcept;

#if defined(NESIMPLE_HAS_SDL)
    [[nodiscard]] bool handle_event(const SDL_Event& event, InputMapping& mapping);
#endif

    void draw(
        std::vector<std::uint32_t>& pixels,
        const InputMapping& mapping
    ) const;

private:
    enum class BindingKind : std::uint8_t {
        Keyboard,
        Gamepad
    };

    struct DropdownOption {
        int value {-1};
        const char* label {"---"};
        bool custom {false};
    };

    [[nodiscard]] static NesButton button_for_row(int row) noexcept;
    [[nodiscard]] static const char* button_name(NesButton button) noexcept;
    [[nodiscard]] static const char* keyboard_name(int scancode);
    [[nodiscard]] static const char* gamepad_button_name(int button);

    [[nodiscard]] static const std::vector<DropdownOption>& keyboard_options();
    [[nodiscard]] static const std::vector<DropdownOption>& gamepad_options();

    [[nodiscard]] const std::vector<DropdownOption>& active_options() const noexcept;

    [[nodiscard]] int row_at(int x, int y) const noexcept;
    [[nodiscard]] int dropdown_option_at(int x, int y) const noexcept;
    [[nodiscard]] bool keyboard_tab_hit(int x, int y) const noexcept;
    [[nodiscard]] bool gamepad_tab_hit(int x, int y) const noexcept;
    [[nodiscard]] bool binding_field_hit(int x, int y) const noexcept;

    void open_dropdown_for_row(int row) noexcept;
    void close_dropdown() noexcept;
    void begin_custom_capture() noexcept;
    void cancel_custom_capture() noexcept;

    void move_selection(int delta) noexcept;
    void move_dropdown_selection(int delta) noexcept;
    void apply_dropdown_selection(InputMapping& mapping) noexcept;
    void bind_keyboard(InputMapping& mapping, int scancode) noexcept;
    void bind_gamepad(InputMapping& mapping, int button) noexcept;

    bool open_ {false};
    BindingKind active_kind_ {BindingKind::Keyboard};

    int selected_row_ {0};

    bool dropdown_open_ {false};
    int dropdown_row_ {0};
    int dropdown_selected_index_ {0};

    bool custom_capture_open_ {false};
};

} // namespace nes