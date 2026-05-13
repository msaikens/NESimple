#include "apu/length_counter.hpp"

#include "apu/apu_tables.hpp"

namespace nes {

void LengthCounter::reset() noexcept {
    enabled_ = false;
    value_ = 0;
}

void LengthCounter::set_enabled(bool enabled) noexcept {
    enabled_ = enabled;

    if (!enabled_) {
        value_ = 0;
    }
}

void LengthCounter::load(u8 length_index) noexcept {
    if (!enabled_) {
        return;
    }

    value_ = apu_tables::kLengthTable[length_index & 0x1FU];
}

void LengthCounter::tick_half_frame(bool halt) noexcept {
    if (halt) {
        return;
    }

    if (value_ > 0U) {
        --value_;
    }
}

bool LengthCounter::enabled() const noexcept {
    return enabled_;
}

bool LengthCounter::active() const noexcept {
    return value_ > 0U;
}

u8 LengthCounter::value() const noexcept {
    return value_;
}

} // namespace nes