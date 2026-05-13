#pragma once

#include <cstdint>

namespace nes {

enum class NesButton : std::uint8_t {
    A,
    B,
    Select,
    Start,
    Up,
    Down,
    Left,
    Right
};

struct ButtonBinding {
    int keyboard_scancode {-1};
    int gamepad_button {-1};
};

struct InputMapping {
    ButtonBinding a {};
    ButtonBinding b {};
    ButtonBinding select {};
    ButtonBinding start {};
    ButtonBinding up {};
    ButtonBinding down {};
    ButtonBinding left {};
    ButtonBinding right {};
};

[[nodiscard]] inline ButtonBinding& binding_for(InputMapping& mapping, NesButton button) noexcept {
    switch (button) {
    case NesButton::A:
        return mapping.a;
    case NesButton::B:
        return mapping.b;
    case NesButton::Select:
        return mapping.select;
    case NesButton::Start:
        return mapping.start;
    case NesButton::Up:
        return mapping.up;
    case NesButton::Down:
        return mapping.down;
    case NesButton::Left:
        return mapping.left;
    case NesButton::Right:
        return mapping.right;
    }

    return mapping.a;
}

[[nodiscard]] inline const ButtonBinding& binding_for(
    const InputMapping& mapping,
    NesButton button
) noexcept {
    switch (button) {
    case NesButton::A:
        return mapping.a;
    case NesButton::B:
        return mapping.b;
    case NesButton::Select:
        return mapping.select;
    case NesButton::Start:
        return mapping.start;
    case NesButton::Up:
        return mapping.up;
    case NesButton::Down:
        return mapping.down;
    case NesButton::Left:
        return mapping.left;
    case NesButton::Right:
        return mapping.right;
    }

    return mapping.a;
}

} // namespace nes