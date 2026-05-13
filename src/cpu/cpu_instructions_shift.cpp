#include "cpu.hpp"

namespace nes {

bool Cpu::handle_shift(u8 opcode, usize& cycles) {
    switch (opcode) {
    case 0x0AU:
        a_ = asl_value(a_);
        cycles = 2U;
        return true;

    case 0x06U: {
        const u16 addr = addr_zero_page();
        bus_.write(addr, asl_value(bus_.read(addr)));
        cycles = 5U;
        return true;
    }

    case 0x16U: {
        const u16 addr = addr_zero_page_x();
        bus_.write(addr, asl_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x0EU: {
        const u16 addr = addr_absolute();
        bus_.write(addr, asl_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x1EU: {
        const u16 addr = addr_absolute_x();
        bus_.write(addr, asl_value(bus_.read(addr)));
        cycles = 7U;
        return true;
    }

    case 0x4AU:
        a_ = lsr_value(a_);
        cycles = 2U;
        return true;

    case 0x46U: {
        const u16 addr = addr_zero_page();
        bus_.write(addr, lsr_value(bus_.read(addr)));
        cycles = 5U;
        return true;
    }

    case 0x56U: {
        const u16 addr = addr_zero_page_x();
        bus_.write(addr, lsr_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x4EU: {
        const u16 addr = addr_absolute();
        bus_.write(addr, lsr_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x5EU: {
        const u16 addr = addr_absolute_x();
        bus_.write(addr, lsr_value(bus_.read(addr)));
        cycles = 7U;
        return true;
    }

    case 0x2AU:
        a_ = rol_value(a_);
        cycles = 2U;
        return true;

    case 0x26U: {
        const u16 addr = addr_zero_page();
        bus_.write(addr, rol_value(bus_.read(addr)));
        cycles = 5U;
        return true;
    }

    case 0x36U: {
        const u16 addr = addr_zero_page_x();
        bus_.write(addr, rol_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x2EU: {
        const u16 addr = addr_absolute();
        bus_.write(addr, rol_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x3EU: {
        const u16 addr = addr_absolute_x();
        bus_.write(addr, rol_value(bus_.read(addr)));
        cycles = 7U;
        return true;
    }

    case 0x6AU:
        a_ = ror_value(a_);
        cycles = 2U;
        return true;

    case 0x66U: {
        const u16 addr = addr_zero_page();
        bus_.write(addr, ror_value(bus_.read(addr)));
        cycles = 5U;
        return true;
    }

    case 0x76U: {
        const u16 addr = addr_zero_page_x();
        bus_.write(addr, ror_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x6EU: {
        const u16 addr = addr_absolute();
        bus_.write(addr, ror_value(bus_.read(addr)));
        cycles = 6U;
        return true;
    }

    case 0x7EU: {
        const u16 addr = addr_absolute_x();
        bus_.write(addr, ror_value(bus_.read(addr)));
        cycles = 7U;
        return true;
    }

    case 0xE6U: {
        const u16 addr = addr_zero_page();
        const u8 value = static_cast<u8>(bus_.read(addr) + 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 5U;
        return true;
    }

    case 0xF6U: {
        const u16 addr = addr_zero_page_x();
        const u8 value = static_cast<u8>(bus_.read(addr) + 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 6U;
        return true;
    }

    case 0xEEU: {
        const u16 addr = addr_absolute();
        const u8 value = static_cast<u8>(bus_.read(addr) + 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 6U;
        return true;
    }

    case 0xFEU: {
        const u16 addr = addr_absolute_x();
        const u8 value = static_cast<u8>(bus_.read(addr) + 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 7U;
        return true;
    }

    case 0xC6U: {
        const u16 addr = addr_zero_page();
        const u8 value = static_cast<u8>(bus_.read(addr) - 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 5U;
        return true;
    }

    case 0xD6U: {
        const u16 addr = addr_zero_page_x();
        const u8 value = static_cast<u8>(bus_.read(addr) - 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 6U;
        return true;
    }

    case 0xCEU: {
        const u16 addr = addr_absolute();
        const u8 value = static_cast<u8>(bus_.read(addr) - 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 6U;
        return true;
    }

    case 0xDEU: {
        const u16 addr = addr_absolute_x();
        const u8 value = static_cast<u8>(bus_.read(addr) - 1U);
        bus_.write(addr, value);
        set_zn(value);
        cycles = 7U;
        return true;
    }

    default:
        return false;
    }
}

} // namespace nes