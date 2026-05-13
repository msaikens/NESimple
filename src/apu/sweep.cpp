#include "apu/sweep.hpp"

namespace nes {

void SweepUnit::reset() noexcept {
    enabled_ = false;
    period_ = 0;
    negate_ = false;
    shift_ = 0;
    divider_ = 0;
    reload_ = false;
}

void SweepUnit::write(u8 value) noexcept {
    enabled_ = (value & 0x80U) != 0U;
    period_ = static_cast<u8>((value >> 4U) & 0x07U);
    negate_ = (value & 0x08U) != 0U;
    shift_ = static_cast<u8>(value & 0x07U);
    reload_ = true;
}

void SweepUnit::restart() noexcept {
    reload_ = true;
}

u16 SweepUnit::target_period(u16 timer, bool pulse1) const noexcept {
    const u16 change = static_cast<u16>(timer >> shift_);

    if (!negate_) {
        return static_cast<u16>(timer + change);
    }

    // NES pulse 1 uses one's complement for negative sweep; pulse 2 uses two's complement.
    return static_cast<u16>(timer - change - (pulse1 ? 1U : 0U));
}

bool SweepUnit::muted(u16 timer) const noexcept {
    if (timer < 8U) {
        return true;
    }

    if (shift_ == 0U) {
        return false;
    }

    return target_period(timer, false) > 0x07FFU;
}

void SweepUnit::tick_half_frame(u16& timer, bool pulse1) noexcept {
    if (divider_ == 0U) {
        if (enabled_ && shift_ > 0U && !muted(timer)) {
            const u16 target = target_period(timer, pulse1);

            if (target <= 0x07FFU) {
                timer = target;
            }
        }

        divider_ = period_;
    } else {
        --divider_;
    }

    if (reload_) {
        reload_ = false;
        divider_ = period_;
    }
}

} // namespace nes