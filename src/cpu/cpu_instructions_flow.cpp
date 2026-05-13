#include "cpu.hpp"

namespace nes {

bool Cpu::handle_flow(u8 opcode, usize& cycles) {
    switch (opcode) {
    case 0x00U:
        pc_ = static_cast<u16>(pc_ + 1U);
        enter_interrupt(0xFFFEU, true);
        cycles = 7U;
        return true;

case 0x40U:
    status_ = pop();
    status_ = static_cast<u8>((status_ | Unused) & ~Break);
    pc_ = pop_word();
    in_nmi_ = false;
    cycles = 6U;
    return true;

    case 0xF0U:
        cycles = branch_if(get_flag(Zero));
        return true;

    case 0xD0U:
        cycles = branch_if(!get_flag(Zero));
        return true;

    case 0x30U:
        cycles = branch_if(get_flag(Negative));
        return true;

    case 0x10U:
        cycles = branch_if(!get_flag(Negative));
        return true;

    case 0x90U:
        cycles = branch_if(!get_flag(Carry));
        return true;

    case 0xB0U:
        cycles = branch_if(get_flag(Carry));
        return true;

    case 0x50U:
        cycles = branch_if(!get_flag(Overflow));
        return true;

    case 0x70U:
        cycles = branch_if(get_flag(Overflow));
        return true;

    case 0x20U: {
        const u16 addr = fetch_word();
        const u16 return_addr = static_cast<u16>(pc_ - 1U);
        push_word(return_addr);
        pc_ = addr;
        cycles = 6U;
        return true;
    }

    case 0x4CU:
        pc_ = fetch_word();
        cycles = 3U;
        return true;

    case 0x6CU:
        pc_ = read_word_bug(fetch_word());
        cycles = 5U;
        return true;

    case 0x60U:
        pc_ = static_cast<u16>(pop_word() + 1U);
        cycles = 6U;
        return true;

    default:
        return false;
    }
}

} // namespace nes