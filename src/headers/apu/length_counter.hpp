#pragma once

#include "common/types.hpp"

namespace nes {

class LengthCounter {
public:
    void reset() noexcept;

    void set_enabled(bool enabled) noexcept;
    void load(u8 length_index) noexcept;
    void tick_half_frame(bool halt) noexcept;

    [[nodiscard]] bool enabled() const noexcept;
    [[nodiscard]] bool active() const noexcept;
    [[nodiscard]] u8 value() const noexcept;

private:
    bool enabled_ {false};
    u8 value_ {0};
};

} // namespace nes