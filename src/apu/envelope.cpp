#include "apu/envelope.hpp"

namespace nes {

void Envelope::reset() noexcept {
    loop_flag_ = false;
    constant_volume_ = false;
    start_flag_ = false;
    volume_period_ = 0;
    divider_ = 0;
    decay_level_ = 0;
}

void Envelope::write_control(u8 value) noexcept {
    loop_flag_ = (value & 0x20U) != 0U;
    constant_volume_ = (value & 0x10U) != 0U;
    volume_period_ = static_cast<u8>(value & 0x0FU);
}

void Envelope::restart() noexcept {
    start_flag_ = true;
}

void Envelope::tick_quarter_frame() noexcept {
    if (start_flag_) {
        start_flag_ = false;
        decay_level_ = 15U;
        divider_ = volume_period_;
        return;
    }

    if (divider_ > 0U) {
        --divider_;
        return;
    }

    divider_ = volume_period_;

    if (decay_level_ > 0U) {
        --decay_level_;
    } else if (loop_flag_) {
        decay_level_ = 15U;
    }
}

u8 Envelope::output_volume() const noexcept {
    return constant_volume_ ? volume_period_ : decay_level_;
}

bool Envelope::loop_flag() const noexcept {
    return loop_flag_;
}

bool Envelope::constant_volume() const noexcept {
    return constant_volume_;
}

u8 Envelope::raw_volume() const noexcept {
    return volume_period_;
}

} // namespace nes