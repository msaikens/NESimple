#include "ui/input_settings_menu.hpp"

#include "ui/ui_draw.hpp"

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int kPanelX = 8;
constexpr int kPanelY = 16;
constexpr int kPanelW = 240;
constexpr int kPanelH = 208;

constexpr int kTitleX = 20;
constexpr int kTitleY = 26;

constexpr int kKeyboardTabX = 20;
constexpr int kGamepadTabX = 112;
constexpr int kTabY = 40;
constexpr int kTabW = 84;
constexpr int kTabH = 13;

constexpr int kButtonX = 22;
constexpr int kFieldX = 82;
constexpr int kFieldW = 150;
constexpr int kHeaderY = 60;
constexpr int kFirstRowY = 74;
constexpr int kRowHeight = 15;

constexpr int kDropdownX = kFieldX;
constexpr int kDropdownY = 74;
constexpr int kDropdownW = kFieldW;
constexpr int kDropdownItemH = 13;
constexpr int kDropdownVisibleItems = 10;

constexpr std::uint32_t kDim = 0xAA000000U;
constexpr std::uint32_t kPanel = 0xEE101820U;
constexpr std::uint32_t kBorder = 0xFFB8B8B8U;
constexpr std::uint32_t kText = 0xFFFFFEFFU;
constexpr std::uint32_t kMuted = 0xFFB8B8B8U;
constexpr std::uint32_t kSelected = 0xFF324860U;
constexpr std::uint32_t kField = 0xFF202C38U;
constexpr std::uint32_t kActiveTab = 0xFF405870U;
constexpr std::uint32_t kInactiveTab = 0xFF1D2833U;
constexpr std::uint32_t kDropdownBg = 0xFF080C10U;
constexpr std::uint32_t kDropdownSelected = 0xFF665522U;
constexpr std::uint32_t kCapturePanel = 0xF0201820U;

void draw_border(
    std::vector<std::uint32_t>& pixels,
    int x,
    int y,
    int w,
    int h,
    std::uint32_t color
) {
    nes::ui::draw_rect(pixels, x, y, w, 1, color);
    nes::ui::draw_rect(pixels, x, y + h - 1, w, 1, color);
    nes::ui::draw_rect(pixels, x, y, 1, h, color);
    nes::ui::draw_rect(pixels, x + w - 1, y, 1, h, color);
}

} // namespace

namespace nes {

bool InputSettingsMenu::open() const noexcept {
    return open_;
}

void InputSettingsMenu::toggle() noexcept {
    open_ = !open_;
    close_dropdown();
    cancel_custom_capture();
}

void InputSettingsMenu::close() noexcept {
    open_ = false;
    close_dropdown();
    cancel_custom_capture();
}

NesButton InputSettingsMenu::button_for_row(int row) noexcept {
    switch (std::clamp(row, 0, 7)) {
    case 0:
        return NesButton::A;
    case 1:
        return NesButton::B;
    case 2:
        return NesButton::Select;
    case 3:
        return NesButton::Start;
    case 4:
        return NesButton::Up;
    case 5:
        return NesButton::Down;
    case 6:
        return NesButton::Left;
    case 7:
        return NesButton::Right;
    default:
        return NesButton::A;
    }
}

const char* InputSettingsMenu::button_name(NesButton button) noexcept {
    switch (button) {
    case NesButton::A:
        return "A";
    case NesButton::B:
        return "B";
    case NesButton::Select:
        return "SELECT";
    case NesButton::Start:
        return "START";
    case NesButton::Up:
        return "UP";
    case NesButton::Down:
        return "DOWN";
    case NesButton::Left:
        return "LEFT";
    case NesButton::Right:
        return "RIGHT";
    }

    return "A";
}

const char* InputSettingsMenu::keyboard_name(int scancode) {
#if defined(NESIMPLE_HAS_SDL)
    if (scancode < 0) {
        return "---";
    }

    const char* name = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
    if (name == nullptr || name[0] == '\0') {
        return "---";
    }

    return name;
#else
    static_cast<void>(scancode);
    return "---";
#endif
}

const char* InputSettingsMenu::gamepad_button_name(int button) {
#if defined(NESIMPLE_HAS_SDL)
    if (button < 0) {
        return "---";
    }

    const char* name = SDL_GetGamepadStringForButton(
        static_cast<SDL_GamepadButton>(button)
    );

    if (name == nullptr || name[0] == '\0') {
        return "---";
    }

    return name;
#else
    static_cast<void>(button);
    return "---";
#endif
}

const std::vector<InputSettingsMenu::DropdownOption>&
InputSettingsMenu::keyboard_options() {
#if defined(NESIMPLE_HAS_SDL)
    static const std::vector<DropdownOption> options {
        {-1, "CUSTOM...", true},
        {-1, "UNBOUND", false},
        {SDL_SCANCODE_X, "X", false},
        {SDL_SCANCODE_Z, "Z", false},
        {SDL_SCANCODE_A, "A", false},
        {SDL_SCANCODE_S, "S", false},
        {SDL_SCANCODE_D, "D", false},
        {SDL_SCANCODE_F, "F", false},
        {SDL_SCANCODE_Q, "Q", false},
        {SDL_SCANCODE_W, "W", false},
        {SDL_SCANCODE_E, "E", false},
        {SDL_SCANCODE_SPACE, "SPACE", false},
        {SDL_SCANCODE_RETURN, "ENTER", false},
        {SDL_SCANCODE_RSHIFT, "RSHIFT", false},
        {SDL_SCANCODE_LSHIFT, "LSHIFT", false},
        {SDL_SCANCODE_UP, "UP", false},
        {SDL_SCANCODE_DOWN, "DOWN", false},
        {SDL_SCANCODE_LEFT, "LEFT", false},
        {SDL_SCANCODE_RIGHT, "RIGHT", false}
    };
#else
    static const std::vector<DropdownOption> options {
        {-1, "CUSTOM...", true},
        {-1, "UNBOUND", false}
    };
#endif

    return options;
}

const std::vector<InputSettingsMenu::DropdownOption>&
InputSettingsMenu::gamepad_options() {
#if defined(NESIMPLE_HAS_SDL)
    static const std::vector<DropdownOption> options {
        {-1, "CUSTOM...", true},
        {-1, "UNBOUND", false},
        {SDL_GAMEPAD_BUTTON_SOUTH, "SOUTH", false},
        {SDL_GAMEPAD_BUTTON_EAST, "EAST", false},
        {SDL_GAMEPAD_BUTTON_WEST, "WEST", false},
        {SDL_GAMEPAD_BUTTON_NORTH, "NORTH", false},
        {SDL_GAMEPAD_BUTTON_BACK, "BACK", false},
        {SDL_GAMEPAD_BUTTON_START, "START", false},
        {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, "L SHOULDER", false},
        {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, "R SHOULDER", false},
        {SDL_GAMEPAD_BUTTON_DPAD_UP, "DPAD UP", false},
        {SDL_GAMEPAD_BUTTON_DPAD_DOWN, "DPAD DOWN", false},
        {SDL_GAMEPAD_BUTTON_DPAD_LEFT, "DPAD LEFT", false},
        {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, "DPAD RIGHT", false}
    };
#else
    static const std::vector<DropdownOption> options {
        {-1, "CUSTOM...", true},
        {-1, "UNBOUND", false}
    };
#endif

    return options;
}

const std::vector<InputSettingsMenu::DropdownOption>&
InputSettingsMenu::active_options() const noexcept {
    if (active_kind_ == BindingKind::Keyboard) {
        return keyboard_options();
    }

    return gamepad_options();
}

int InputSettingsMenu::row_at(int x, int y) const noexcept {
    if (x < kButtonX || x >= kPanelX + kPanelW) {
        return -1;
    }

    if (y < kFirstRowY) {
        return -1;
    }

    const int row = (y - kFirstRowY) / kRowHeight;
    if (row < 0 || row > 7) {
        return -1;
    }

    return row;
}

bool InputSettingsMenu::keyboard_tab_hit(int x, int y) const noexcept {
    return x >= kKeyboardTabX &&
           x < kKeyboardTabX + kTabW &&
           y >= kTabY &&
           y < kTabY + kTabH;
}

bool InputSettingsMenu::gamepad_tab_hit(int x, int y) const noexcept {
    return x >= kGamepadTabX &&
           x < kGamepadTabX + kTabW &&
           y >= kTabY &&
           y < kTabY + kTabH;
}

bool InputSettingsMenu::binding_field_hit(int x, int y) const noexcept {
    const int row = row_at(x, y);
    if (row < 0) {
        return false;
    }

    const int row_y = kFirstRowY + row * kRowHeight;
    return x >= kFieldX &&
           x < kFieldX + kFieldW &&
           y >= row_y - 2 &&
           y < row_y + 11;
}

int InputSettingsMenu::dropdown_option_at(int x, int y) const noexcept {
    if (!dropdown_open_) {
        return -1;
    }

    if (x < kDropdownX || x >= kDropdownX + kDropdownW) {
        return -1;
    }

    if (y < kDropdownY) {
        return -1;
    }

    const int index = (y - kDropdownY) / kDropdownItemH;
    const int option_count = static_cast<int>(active_options().size());
    const int visible_count = std::min(option_count, kDropdownVisibleItems);

    if (index < 0 || index >= visible_count) {
        return -1;
    }

    return index;
}

void InputSettingsMenu::open_dropdown_for_row(int row) noexcept {
    selected_row_ = std::clamp(row, 0, 7);
    dropdown_row_ = selected_row_;
    dropdown_selected_index_ = 0;
    dropdown_open_ = true;
    custom_capture_open_ = false;
}

void InputSettingsMenu::close_dropdown() noexcept {
    dropdown_open_ = false;
    dropdown_selected_index_ = 0;
}

void InputSettingsMenu::begin_custom_capture() noexcept {
    custom_capture_open_ = true;
    dropdown_open_ = false;
}

void InputSettingsMenu::cancel_custom_capture() noexcept {
    custom_capture_open_ = false;
}

void InputSettingsMenu::move_selection(int delta) noexcept {
    selected_row_ = std::clamp(selected_row_ + delta, 0, 7);
}

void InputSettingsMenu::move_dropdown_selection(int delta) noexcept {
    const int option_count = static_cast<int>(active_options().size());
    if (option_count <= 0) {
        dropdown_selected_index_ = 0;
        return;
    }

    const int visible_count = std::min(option_count, kDropdownVisibleItems);
    dropdown_selected_index_ =
        std::clamp(dropdown_selected_index_ + delta, 0, visible_count - 1);
}

void InputSettingsMenu::bind_keyboard(InputMapping& mapping, int scancode) noexcept {
    ButtonBinding& binding = binding_for(mapping, button_for_row(dropdown_row_));
    binding.keyboard_scancode = scancode;
    custom_capture_open_ = false;
    close_dropdown();
}

void InputSettingsMenu::bind_gamepad(InputMapping& mapping, int button) noexcept {
    ButtonBinding& binding = binding_for(mapping, button_for_row(dropdown_row_));
    binding.gamepad_button = button;
    custom_capture_open_ = false;
    close_dropdown();
}

void InputSettingsMenu::apply_dropdown_selection(InputMapping& mapping) noexcept {
    const auto& options = active_options();

    if (dropdown_selected_index_ < 0 ||
        dropdown_selected_index_ >= static_cast<int>(options.size())) {
        close_dropdown();
        return;
    }

    const DropdownOption& option = options[dropdown_selected_index_];

    if (option.custom) {
        begin_custom_capture();
        return;
    }

    ButtonBinding& binding = binding_for(mapping, button_for_row(dropdown_row_));

    if (active_kind_ == BindingKind::Keyboard) {
        binding.keyboard_scancode = option.value;
    } else {
        binding.gamepad_button = option.value;
    }

    close_dropdown();
}

#if defined(NESIMPLE_HAS_SDL)
bool InputSettingsMenu::handle_event(const SDL_Event& event, InputMapping& mapping) {
    if (!open_) {
        if (event.type == SDL_EVENT_KEY_DOWN &&
            !event.key.repeat &&
            event.key.key == SDLK_F2) {
            toggle();
            return true;
        }

        return false;
    }

    if (custom_capture_open_) {
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            if (event.key.key == SDLK_ESCAPE) {
                cancel_custom_capture();
                return true;
            }

            if (active_kind_ == BindingKind::Keyboard) {
                bind_keyboard(mapping, static_cast<int>(event.key.scancode));
            }

            return true;
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            if (active_kind_ == BindingKind::Gamepad) {
                bind_gamepad(mapping, static_cast<int>(event.gbutton.button));
            }

            return true;
        }

        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (dropdown_open_) {
            switch (event.key.key) {
            case SDLK_ESCAPE:
                close_dropdown();
                return true;

            case SDLK_UP:
                move_dropdown_selection(-1);
                return true;

            case SDLK_DOWN:
                move_dropdown_selection(1);
                return true;

            case SDLK_RETURN:
            case SDLK_SPACE:
                apply_dropdown_selection(mapping);
                return true;

            default:
                return true;
            }
        }

        switch (event.key.key) {
        case SDLK_F2:
        case SDLK_ESCAPE:
            close();
            return true;

        case SDLK_TAB:
        case SDLK_G:
            active_kind_ = BindingKind::Gamepad;
            close_dropdown();
            return true;

        case SDLK_K:
            active_kind_ = BindingKind::Keyboard;
            close_dropdown();
            return true;

        case SDLK_R:
            mapping = {};
            close_dropdown();
            cancel_custom_capture();
            return true;

        case SDLK_UP:
            move_selection(-1);
            return true;

        case SDLK_DOWN:
            move_selection(1);
            return true;

        case SDLK_RETURN:
        case SDLK_SPACE:
            open_dropdown_for_row(selected_row_);
            return true;

        default:
            return true;
        }
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        const int x = static_cast<int>(event.button.x);
        const int y = static_cast<int>(event.button.y);

        if (dropdown_open_) {
            const int option = dropdown_option_at(x, y);
            if (option >= 0) {
                dropdown_selected_index_ = option;
                apply_dropdown_selection(mapping);
                return true;
            }

            close_dropdown();
            return true;
        }

        if (keyboard_tab_hit(x, y)) {
            active_kind_ = BindingKind::Keyboard;
            close_dropdown();
            cancel_custom_capture();
            return true;
        }

        if (gamepad_tab_hit(x, y)) {
            active_kind_ = BindingKind::Gamepad;
            close_dropdown();
            cancel_custom_capture();
            return true;
        }

        if (binding_field_hit(x, y)) {
            const int row = row_at(x, y);
            if (row >= 0) {
                open_dropdown_for_row(row);
            }

            return true;
        }

        const int row = row_at(x, y);
        if (row >= 0) {
            selected_row_ = row;
            return true;
        }

        return true;
    }

    return true;
}
#endif

void InputSettingsMenu::draw(
    std::vector<std::uint32_t>& pixels,
    const InputMapping& mapping
) const {
    if (!open_) {
        return;
    }

    ui::draw_rect(pixels, 0, 0, 256, 240, kDim);

    ui::draw_rect(pixels, kPanelX, kPanelY, kPanelW, kPanelH, kPanel);
    draw_border(pixels, kPanelX, kPanelY, kPanelW, kPanelH, kBorder);

    ui::draw_text(pixels, kTitleX, kTitleY, "INPUT SETTINGS", kText);

    ui::draw_rect(
        pixels,
        kKeyboardTabX,
        kTabY,
        kTabW,
        kTabH,
        active_kind_ == BindingKind::Keyboard ? kActiveTab : kInactiveTab
    );
    draw_border(pixels, kKeyboardTabX, kTabY, kTabW, kTabH, kBorder);
    ui::draw_text(pixels, kKeyboardTabX + 8, kTabY + 3, "KEYBOARD", kText);

    ui::draw_rect(
        pixels,
        kGamepadTabX,
        kTabY,
        kTabW,
        kTabH,
        active_kind_ == BindingKind::Gamepad ? kActiveTab : kInactiveTab
    );
    draw_border(pixels, kGamepadTabX, kTabY, kTabW, kTabH, kBorder);
    ui::draw_text(pixels, kGamepadTabX + 14, kTabY + 3, "GAMEPAD", kText);

    ui::draw_text(pixels, kButtonX, kHeaderY, "BUTTON", kMuted);
    ui::draw_text(
        pixels,
        kFieldX + 4,
        kHeaderY,
        active_kind_ == BindingKind::Keyboard ? "KEY" : "PAD",
        kMuted
    );

    for (int row = 0; row < 8; ++row) {
        const NesButton button = button_for_row(row);
        const ButtonBinding& binding = binding_for(mapping, button);
        const int y = kFirstRowY + row * kRowHeight;

        if (row == selected_row_) {
            ui::draw_rect(pixels, kButtonX - 4, y - 3, 214, 13, kSelected);
        }

        ui::draw_text(pixels, kButtonX, y, button_name(button), kText);

        ui::draw_rect(pixels, kFieldX, y - 3, kFieldW, 13, kField);
        draw_border(pixels, kFieldX, y - 3, kFieldW, 13, kBorder);

        const std::string name = active_kind_ == BindingKind::Keyboard
            ? keyboard_name(binding.keyboard_scancode)
            : gamepad_button_name(binding.gamepad_button);

        const std::string label = ui::uppercase_text(ui::limit_text(name, 16U));
        ui::draw_text(pixels, kFieldX + 4, y, label, kText);

        ui::draw_text(pixels, kFieldX + kFieldW - 11, y, "V", kMuted);
    }

    ui::draw_text(pixels, 20, 202, "CLICK FIELD TO OPEN DROPDOWN", kMuted);
    ui::draw_text(pixels, 20, 213, "F2/ESC CLOSE  R CLEAR  TAB PAD", kMuted);

    if (dropdown_open_) {
        const auto& options = active_options();
        const int visible_count = std::min(
            static_cast<int>(options.size()),
            kDropdownVisibleItems
        );

        ui::draw_rect(
            pixels,
            kDropdownX,
            kDropdownY,
            kDropdownW,
            visible_count * kDropdownItemH + 2,
            kDropdownBg
        );
        draw_border(
            pixels,
            kDropdownX,
            kDropdownY,
            kDropdownW,
            visible_count * kDropdownItemH + 2,
            kBorder
        );

        for (int index = 0; index < visible_count; ++index) {
            const int y = kDropdownY + 1 + index * kDropdownItemH;

            if (index == dropdown_selected_index_) {
                ui::draw_rect(
                    pixels,
                    kDropdownX + 1,
                    y,
                    kDropdownW - 2,
                    kDropdownItemH,
                    kDropdownSelected
                );
            }

            const std::string label = ui::uppercase_text(
                ui::limit_text(options[index].label, 17U)
            );

            ui::draw_text(pixels, kDropdownX + 5, y + 3, label, kText);
        }
    }

    if (custom_capture_open_) {
        ui::draw_rect(pixels, 30, 88, 196, 58, kCapturePanel);
        draw_border(pixels, 30, 88, 196, 58, kBorder);

        std::ostringstream line;
        line << "PRESS "
             << (active_kind_ == BindingKind::Keyboard ? "ANY KEY" : "PAD BUTTON");

        ui::draw_text(pixels, 45, 102, ui::uppercase_text(line.str()), kText);

        std::ostringstream target;
        target << "FOR " << button_name(button_for_row(dropdown_row_));
        ui::draw_text(pixels, 45, 114, ui::uppercase_text(target.str()), kText);

        ui::draw_text(pixels, 45, 130, "ESC CANCEL", kMuted);
    }
}

} // namespace nes