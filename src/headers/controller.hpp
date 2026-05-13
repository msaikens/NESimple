#pragma once

#include "common/types.hpp"

namespace nes {

class Controller {
public:
    enum Button : u8 {
        A      = 1U << 0U,
        B      = 1U << 1U,
        Select = 1U << 2U,
        Start  = 1U << 3U,
        Up     = 1U << 4U,
        Down   = 1U << 5U,
        Left   = 1U << 6U,
        Right  = 1U << 7U
    };

    void set_buttons(u8 state);
    void write(u8 value);
    u8 read();

private:
    u8 state_ {0};
    u8 shift_ {0};
    bool strobe_ {false};
};

} // namespace nes