#include "cpu.hpp"

namespace nes {

bool Cpu::handle_stack(u8 opcode, usize& cycles) {
    switch (opcode) {
    case 0xAAU:
        x_ = a_;
        set_zn(x_);
        cycles = 2U;
        return true;

    case 0xA8U:
        y_ = a_;
        set_zn(y_);
        cycles = 2U;
        return true;

    case 0x8AU:
        a_ = x_;
        set_zn(a_);
        cycles = 2U;
        return true;

    case 0x98U:
        a_ = y_;
        set_zn(a_);
        cycles = 2U;
        return true;

    case 0xBAU:
        x_ = sp_;
        set_zn(x_);
        cycles = 2U;
        return true;

    case 0x9AU:
        sp_ = x_;
        cycles = 2U;
        return true;

    case 0xE8U:
        x_ = static_cast<u8>(x_ + 1U);
        set_zn(x_);
        cycles = 2U;
        return true;

    case 0xC8U:
        y_ = static_cast<u8>(y_ + 1U);
        set_zn(y_);
        cycles = 2U;
        return true;

    case 0xCAU:
        x_ = static_cast<u8>(x_ - 1U);
        set_zn(x_);
        cycles = 2U;
        return true;

    case 0x88U:
        y_ = static_cast<u8>(y_ - 1U);
        set_zn(y_);
        cycles = 2U;
        return true;

    case 0x48U:
        push(a_);
        cycles = 3U;
        return true;

    case 0x68U:
        a_ = pop();
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x08U: {
        const u8 pushed = static_cast<u8>(status_ | Break | Unused);
        push(pushed);
        cycles = 3U;
        return true;
    }

    case 0x28U:
        status_ = pop();
        status_ = static_cast<u8>((status_ | Unused) & ~Break);
        cycles = 4U;
        return true;

    default:
        return false;
    }
}

} // namespace nes