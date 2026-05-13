#pragma once

#include "common/types.hpp"

namespace nes {

class Envelope {
public:
    void reset() noexcept;

    void write_control(u8 value) noexcept;
    void restart() noexcept;
    void tick_quarter_frame() noexcept;

    [[nodiscard]] u8 output_volume() const noexcept;
    [[nodiscard]] bool loop_flag() const noexcept;
    [[nodiscard]] bool constant_volume() const noexcept;
    [[nodiscard]] u8 raw_volume() const noexcept;

private:
    bool loop_flag_ {false};
    bool constant_volume_ {false};
    bool start_flag_ {false};

    u8 volume_period_ {0};
    u8 divider_ {0};
    u8 decay_level_ {0};
};

} // namespace nes