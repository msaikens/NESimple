#include "cpu.hpp"

namespace nes {

bool Cpu::handle_load_store(u8 opcode, usize& cycles) {
    switch (opcode) {
    case 0xEAU:
        cycles = 2U;
        return true;

    case 0xA9U:
        a_ = fetch_byte();
        set_zn(a_);
        cycles = 2U;
        return true;

    case 0xA5U:
        a_ = bus_.read(addr_zero_page());
        set_zn(a_);
        cycles = 3U;
        return true;

    case 0xB5U:
        a_ = bus_.read(addr_zero_page_x());
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0xADU:
        a_ = bus_.read(addr_absolute());
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0xBDU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        a_ = bus_.read(addr);
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xB9U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        a_ = bus_.read(addr);
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xA1U:
        a_ = bus_.read(addr_indirect_x());
        set_zn(a_);
        cycles = 6U;
        return true;

    case 0xB1U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        a_ = bus_.read(addr);
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    case 0xA2U:
        x_ = fetch_byte();
        set_zn(x_);
        cycles = 2U;
        return true;

    case 0xA6U:
        x_ = bus_.read(addr_zero_page());
        set_zn(x_);
        cycles = 3U;
        return true;

    case 0xB6U:
        x_ = bus_.read(addr_zero_page_y());
        set_zn(x_);
        cycles = 4U;
        return true;

    case 0xAEU:
        x_ = bus_.read(addr_absolute());
        set_zn(x_);
        cycles = 4U;
        return true;

    case 0xBEU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        x_ = bus_.read(addr);
        set_zn(x_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xA0U:
        y_ = fetch_byte();
        set_zn(y_);
        cycles = 2U;
        return true;

    case 0xA4U:
        y_ = bus_.read(addr_zero_page());
        set_zn(y_);
        cycles = 3U;
        return true;

    case 0xB4U:
        y_ = bus_.read(addr_zero_page_x());
        set_zn(y_);
        cycles = 4U;
        return true;

    case 0xACU:
        y_ = bus_.read(addr_absolute());
        set_zn(y_);
        cycles = 4U;
        return true;

    case 0xBCU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        y_ = bus_.read(addr);
        set_zn(y_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x85U:
        bus_.write(addr_zero_page(), a_);
        cycles = 3U;
        return true;

    case 0x95U:
        bus_.write(addr_zero_page_x(), a_);
        cycles = 4U;
        return true;

    case 0x8DU:
        bus_.write(addr_absolute(), a_);
        cycles = 4U;
        return true;

    case 0x9DU:
        bus_.write(addr_absolute_x(), a_);
        cycles = 5U;
        return true;

    case 0x99U:
        bus_.write(addr_absolute_y(), a_);
        cycles = 5U;
        return true;

    case 0x81U:
        bus_.write(addr_indirect_x(), a_);
        cycles = 6U;
        return true;

    case 0x91U:
        bus_.write(addr_indirect_y(), a_);
        cycles = 6U;
        return true;

    case 0x86U:
        bus_.write(addr_zero_page(), x_);
        cycles = 3U;
        return true;

    case 0x96U:
        bus_.write(addr_zero_page_y(), x_);
        cycles = 4U;
        return true;

    case 0x8EU:
        bus_.write(addr_absolute(), x_);
        cycles = 4U;
        return true;

    case 0x84U:
        bus_.write(addr_zero_page(), y_);
        cycles = 3U;
        return true;

    case 0x94U:
        bus_.write(addr_zero_page_x(), y_);
        cycles = 4U;
        return true;

    case 0x8CU:
        bus_.write(addr_absolute(), y_);
        cycles = 4U;
        return true;

    default:
        return false;
    }
}

} // namespace nes