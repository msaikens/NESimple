#pragma once

#include "common/types.hpp"

namespace nes {

class SweepUnit {
public:
    void reset() noexcept;

    void write(u8 value) noexcept;
    void restart() noexcept;
    void tick_half_frame(u16& timer, bool pulse1) noexcept;

    [[nodiscard]] bool muted(u16 timer) const noexcept;

private:
    bool enabled_ {false};
    u8 period_ {0};
    bool negate_ {false};
    u8 shift_ {0};

    u8 divider_ {0};
    bool reload_ {false};

    [[nodiscard]] u16 target_period(u16 timer, bool pulse1) const noexcept;
};

} // namespace nes