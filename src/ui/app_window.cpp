#include "ui/app_window.hpp"

#include "controller.hpp"
#include "ui/input_settings_menu.hpp"
#include "ui/nes_palette.hpp"
#include "ui/overlay.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#if defined(NESIMPLE_HAS_SDL)
#include <SDL3/SDL.h>
#endif

namespace {

using nes::u8;
using nes::usize;

#if defined(NESIMPLE_HAS_SDL)

nes::InputMapping default_input_mapping() noexcept {
    nes::InputMapping mapping {};

    mapping.a.keyboard_scancode = SDL_SCANCODE_X;
    mapping.b.keyboard_scancode = SDL_SCANCODE_Z;
    mapping.select.keyboard_scancode = SDL_SCANCODE_RSHIFT;
    mapping.start.keyboard_scancode = SDL_SCANCODE_RETURN;
    mapping.up.keyboard_scancode = SDL_SCANCODE_UP;
    mapping.down.keyboard_scancode = SDL_SCANCODE_DOWN;
    mapping.left.keyboard_scancode = SDL_SCANCODE_LEFT;
    mapping.right.keyboard_scancode = SDL_SCANCODE_RIGHT;

    mapping.a.gamepad_button = SDL_GAMEPAD_BUTTON_SOUTH;
    mapping.b.gamepad_button = SDL_GAMEPAD_BUTTON_WEST;
    mapping.select.gamepad_button = SDL_GAMEPAD_BUTTON_BACK;
    mapping.start.gamepad_button = SDL_GAMEPAD_BUTTON_START;
    mapping.up.gamepad_button = SDL_GAMEPAD_BUTTON_DPAD_UP;
    mapping.down.gamepad_button = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    mapping.left.gamepad_button = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
    mapping.right.gamepad_button = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;

    return mapping;
}

bool keyboard_binding_down(const bool* keys, const nes::ButtonBinding& binding) {
    if (keys == nullptr || binding.keyboard_scancode < 0) {
        return false;
    }

    return keys[binding.keyboard_scancode];
}

bool gamepad_binding_down(SDL_Gamepad* pad, const nes::ButtonBinding& binding) {
    if (pad == nullptr || binding.gamepad_button < 0) {
        return false;
    }

    return SDL_GetGamepadButton(
        pad,
        static_cast<SDL_GamepadButton>(binding.gamepad_button)
    );
}

SDL_Event to_logical_mouse_event(const SDL_Event& event, SDL_Window* window) {
    SDL_Event logical = event;

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.type != SDL_EVENT_MOUSE_BUTTON_UP) {
        return logical;
    }

    int window_w = 0;
    int window_h = 0;
    SDL_GetWindowSize(window, &window_w, &window_h);

    if (window_w <= 0 || window_h <= 0) {
        return logical;
    }

    logical.button.x =
        event.button.x * 256.0F / static_cast<float>(window_w);

    logical.button.y =
        event.button.y * 240.0F / static_cast<float>(window_h);

    return logical;
}

#else

nes::InputMapping default_input_mapping() noexcept {
    return {};
}

#endif

} // namespace

namespace nes {

AppWindow::AppWindow()
    : input_settings_menu_(std::make_unique<InputSettingsMenu>()) {
    input_mapping_ = default_input_mapping();

#if defined(NESIMPLE_HAS_SDL)
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        return;
    }

    SDL_Window* window = SDL_CreateWindow(
        "NESimple",
        256 * 3,
        240 * 3,
        SDL_WINDOW_RESIZABLE
    );

    if (window == nullptr) {
        SDL_Quit();
        return;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        256,
        240
    );

    if (texture == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    window_ = window;
    renderer_ = renderer;
    texture_ = texture;
    available_ = true;

    try_open_first_gamepad();
#endif
}

AppWindow::~AppWindow() {
#if defined(NESIMPLE_HAS_SDL)
    close_gamepad();

    if (texture_ != nullptr) {
        SDL_DestroyTexture(static_cast<SDL_Texture*>(texture_));
    }

    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(static_cast<SDL_Renderer*>(renderer_));
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(static_cast<SDL_Window*>(window_));
    }

    if (available_) {
        SDL_Quit();
    }
#endif
}

bool AppWindow::available() const noexcept {
    return available_;
}

bool AppWindow::has_gamepad() const noexcept {
    return gamepad_ != nullptr;
}

bool AppWindow::settings_open() const noexcept {
    return input_settings_menu_ != nullptr && input_settings_menu_->open();
}

bool AppWindow::process_events() {
#if defined(NESIMPLE_HAS_SDL)
    pause_pressed_ = false;
    reset_pressed_ = false;
    step_frame_pressed_ = false;
    frame_counter_toggle_pressed_ = false;

    SDL_Event event {};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }

        SDL_Event logical_event = event;

        if (window_ != nullptr) {
            logical_event = to_logical_mouse_event(
                event,
                static_cast<SDL_Window*>(window_)
            );
        }

        if (input_settings_menu_ != nullptr &&
            input_settings_menu_->handle_event(logical_event, input_mapping_)) {
            continue;
        }

        switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            if (!event.key.repeat) {
                if (event.key.key == SDLK_ESCAPE) {
                    return false;
                }

                if (event.key.key == SDLK_P) {
                    pause_pressed_ = true;
                } else if (event.key.key == SDLK_R) {
                    reset_pressed_ = true;
                } else if (event.key.key == SDLK_N) {
                    step_frame_pressed_ = true;
                } else if (event.key.key == SDLK_F3) {
                    frame_counter_toggle_pressed_ = true;
                }
            }
            break;

        case SDL_EVENT_GAMEPAD_ADDED:
            if (gamepad_ == nullptr) {
                try_open_first_gamepad();
            }
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            if (gamepad_ != nullptr) {
                SDL_Gamepad* current = static_cast<SDL_Gamepad*>(gamepad_);

                if (SDL_GetGamepadID(current) == event.gdevice.which) {
                    close_gamepad();
                }
            }

            try_open_first_gamepad();
            break;

        default:
            break;
        }
    }
#endif

    return true;
}

u8 AppWindow::poll_controller_state() const {
    u8 buttons = 0;

#if defined(NESIMPLE_HAS_SDL)
    if (settings_open()) {
        return 0;
    }

    SDL_PumpEvents();

    const bool* keys = SDL_GetKeyboardState(nullptr);
    SDL_Gamepad* pad = static_cast<SDL_Gamepad*>(gamepad_);

    const auto apply_button =
        [&](const ButtonBinding& binding, u8 button) {
            if (keyboard_binding_down(keys, binding) ||
                gamepad_binding_down(pad, binding)) {
                buttons = static_cast<u8>(buttons | button);
            }
        };

    apply_button(input_mapping_.a, static_cast<u8>(Controller::A));
    apply_button(input_mapping_.b, static_cast<u8>(Controller::B));
    apply_button(input_mapping_.select, static_cast<u8>(Controller::Select));
    apply_button(input_mapping_.start, static_cast<u8>(Controller::Start));
    apply_button(input_mapping_.up, static_cast<u8>(Controller::Up));
    apply_button(input_mapping_.down, static_cast<u8>(Controller::Down));
    apply_button(input_mapping_.left, static_cast<u8>(Controller::Left));
    apply_button(input_mapping_.right, static_cast<u8>(Controller::Right));

    if (pad != nullptr) {
        constexpr Sint16 kDeadzone = 16000;

        const Sint16 left_x = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
        const Sint16 left_y = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);

        if (std::abs(static_cast<int>(left_x)) >= kDeadzone) {
            if (left_x < 0) {
                buttons = static_cast<u8>(buttons | Controller::Left);
            } else {
                buttons = static_cast<u8>(buttons | Controller::Right);
            }
        }

        if (std::abs(static_cast<int>(left_y)) >= kDeadzone) {
            if (left_y < 0) {
                buttons = static_cast<u8>(buttons | Controller::Up);
            } else {
                buttons = static_cast<u8>(buttons | Controller::Down);
            }
        }
    }
#endif

    return buttons;
}

bool AppWindow::consume_pause_pressed() noexcept {
    const bool pressed = pause_pressed_;
    pause_pressed_ = false;
    return pressed;
}

bool AppWindow::consume_reset_pressed() noexcept {
    const bool pressed = reset_pressed_;
    reset_pressed_ = false;
    return pressed;
}

bool AppWindow::consume_step_frame_pressed() noexcept {
    const bool pressed = step_frame_pressed_;
    step_frame_pressed_ = false;
    return pressed;
}

bool AppWindow::consume_frame_counter_toggle_pressed() noexcept {
    const bool pressed = frame_counter_toggle_pressed_;
    frame_counter_toggle_pressed_ = false;
    return pressed;
}

const InputMapping& AppWindow::input_mapping() const noexcept {
    return input_mapping_;
}

void AppWindow::set_input_mapping(const InputMapping& mapping) noexcept {
    input_mapping_ = mapping;
}

void AppWindow::reset_input_mapping_to_defaults() noexcept {
    input_mapping_ = default_input_mapping();
}

void AppWindow::set_title(const std::string& title) {
#if defined(NESIMPLE_HAS_SDL)
    if (available_) {
        SDL_SetWindowTitle(static_cast<SDL_Window*>(window_), title.c_str());
    }
#else
    static_cast<void>(title);
#endif
}

void AppWindow::present_frame(
    const std::array<u8, 256U * 240U>& frame,
    const OverlayState& overlay
) {
#if defined(NESIMPLE_HAS_SDL)
    if (!available_) {
        return;
    }

    std::vector<std::uint32_t> pixels(256U * 240U, 0U);

    for (usize i = 0; i < frame.size(); ++i) {
        pixels[i] = nes_color_to_argb(frame[i]);
    }

    ui::draw_overlay(pixels, overlay);

    if (input_settings_menu_ != nullptr) {
        input_settings_menu_->draw(pixels, input_mapping_);
    }

    SDL_UpdateTexture(
        static_cast<SDL_Texture*>(texture_),
        nullptr,
        pixels.data(),
        256 * static_cast<int>(sizeof(std::uint32_t))
    );

    SDL_RenderClear(static_cast<SDL_Renderer*>(renderer_));
    SDL_RenderTexture(
        static_cast<SDL_Renderer*>(renderer_),
        static_cast<SDL_Texture*>(texture_),
        nullptr,
        nullptr
    );
    SDL_RenderPresent(static_cast<SDL_Renderer*>(renderer_));
#else
    static_cast<void>(frame);
    static_cast<void>(overlay);
#endif
}

void AppWindow::try_open_first_gamepad() {
#if defined(NESIMPLE_HAS_SDL)
    if (gamepad_ != nullptr) {
        return;
    }

    int gamepad_count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepad_count);

    if (gamepads == nullptr) {
        return;
    }

    for (int i = 0; i < gamepad_count; ++i) {
        SDL_Gamepad* candidate = SDL_OpenGamepad(gamepads[i]);

        if (candidate != nullptr) {
            gamepad_ = candidate;
            break;
        }
    }

    SDL_free(gamepads);
#endif
}

void AppWindow::close_gamepad() noexcept {
#if defined(NESIMPLE_HAS_SDL)
    if (gamepad_ != nullptr) {
        SDL_CloseGamepad(static_cast<SDL_Gamepad*>(gamepad_));
        gamepad_ = nullptr;
    }
#endif
}

} // namespace nes