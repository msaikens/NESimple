#include "cpu.hpp"

namespace nes {

bool Cpu::handle_logic(u8 opcode, usize& cycles) {
    switch (opcode) {
    case 0x29U:
        a_ = static_cast<u8>(a_ & fetch_byte());
        set_zn(a_);
        cycles = 2U;
        return true;

    case 0x25U:
        a_ = static_cast<u8>(a_ & bus_.read(addr_zero_page()));
        set_zn(a_);
        cycles = 3U;
        return true;

    case 0x35U:
        a_ = static_cast<u8>(a_ & bus_.read(addr_zero_page_x()));
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x2DU:
        a_ = static_cast<u8>(a_ & bus_.read(addr_absolute()));
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x3DU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        a_ = static_cast<u8>(a_ & bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x39U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        a_ = static_cast<u8>(a_ & bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x21U:
        a_ = static_cast<u8>(a_ & bus_.read(addr_indirect_x()));
        set_zn(a_);
        cycles = 6U;
        return true;

    case 0x31U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        a_ = static_cast<u8>(a_ & bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    case 0x09U:
        a_ = static_cast<u8>(a_ | fetch_byte());
        set_zn(a_);
        cycles = 2U;
        return true;

    case 0x05U:
        a_ = static_cast<u8>(a_ | bus_.read(addr_zero_page()));
        set_zn(a_);
        cycles = 3U;
        return true;

    case 0x15U:
        a_ = static_cast<u8>(a_ | bus_.read(addr_zero_page_x()));
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x0DU:
        a_ = static_cast<u8>(a_ | bus_.read(addr_absolute()));
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x1DU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        a_ = static_cast<u8>(a_ | bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x19U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        a_ = static_cast<u8>(a_ | bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x01U:
        a_ = static_cast<u8>(a_ | bus_.read(addr_indirect_x()));
        set_zn(a_);
        cycles = 6U;
        return true;

    case 0x11U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        a_ = static_cast<u8>(a_ | bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    case 0x49U:
        a_ = static_cast<u8>(a_ ^ fetch_byte());
        set_zn(a_);
        cycles = 2U;
        return true;

    case 0x45U:
        a_ = static_cast<u8>(a_ ^ bus_.read(addr_zero_page()));
        set_zn(a_);
        cycles = 3U;
        return true;

    case 0x55U:
        a_ = static_cast<u8>(a_ ^ bus_.read(addr_zero_page_x()));
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x4DU:
        a_ = static_cast<u8>(a_ ^ bus_.read(addr_absolute()));
        set_zn(a_);
        cycles = 4U;
        return true;

    case 0x5DU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        a_ = static_cast<u8>(a_ ^ bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x59U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        a_ = static_cast<u8>(a_ ^ bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x41U:
        a_ = static_cast<u8>(a_ ^ bus_.read(addr_indirect_x()));
        set_zn(a_);
        cycles = 6U;
        return true;

    case 0x51U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        a_ = static_cast<u8>(a_ ^ bus_.read(addr));
        set_zn(a_);
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    case 0xC9U:
        compare(a_, fetch_byte());
        cycles = 2U;
        return true;

    case 0xC5U:
        compare(a_, bus_.read(addr_zero_page()));
        cycles = 3U;
        return true;

    case 0xD5U:
        compare(a_, bus_.read(addr_zero_page_x()));
        cycles = 4U;
        return true;

    case 0xCDU:
        compare(a_, bus_.read(addr_absolute()));
        cycles = 4U;
        return true;

    case 0xDDU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        compare(a_, bus_.read(addr));
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xD9U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        compare(a_, bus_.read(addr));
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xC1U:
        compare(a_, bus_.read(addr_indirect_x()));
        cycles = 6U;
        return true;

    case 0xD1U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        compare(a_, bus_.read(addr));
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    case 0xE0U:
        compare(x_, fetch_byte());
        cycles = 2U;
        return true;

    case 0xE4U:
        compare(x_, bus_.read(addr_zero_page()));
        cycles = 3U;
        return true;

    case 0xECU:
        compare(x_, bus_.read(addr_absolute()));
        cycles = 4U;
        return true;

    case 0xC0U:
        compare(y_, fetch_byte());
        cycles = 2U;
        return true;

    case 0xC4U:
        compare(y_, bus_.read(addr_zero_page()));
        cycles = 3U;
        return true;

    case 0xCCU:
        compare(y_, bus_.read(addr_absolute()));
        cycles = 4U;
        return true;

    case 0x24U: {
        const u8 value = bus_.read(addr_zero_page());
        set_flag(Zero, static_cast<u8>(a_ & value) == 0U);
        set_flag(Overflow, (value & 0x40U) != 0U);
        set_flag(Negative, (value & 0x80U) != 0U);
        cycles = 3U;
        return true;
    }

    case 0x2CU: {
        const u8 value = bus_.read(addr_absolute());
        set_flag(Zero, static_cast<u8>(a_ & value) == 0U);
        set_flag(Overflow, (value & 0x40U) != 0U);
        set_flag(Negative, (value & 0x80U) != 0U);
        cycles = 4U;
        return true;
    }

    case 0x18U:
        set_flag(Carry, false);
        cycles = 2U;
        return true;

    case 0x38U:
        set_flag(Carry, true);
        cycles = 2U;
        return true;

    case 0x58U:
        set_flag(InterruptDisable, false);
        cycles = 2U;
        return true;

    case 0x78U:
        set_flag(InterruptDisable, true);
        cycles = 2U;
        return true;

    case 0xD8U:
        set_flag(Decimal, false);
        cycles = 2U;
        return true;

    case 0xF8U:
        set_flag(Decimal, true);
        cycles = 2U;
        return true;

    case 0xB8U:
        set_flag(Overflow, false);
        cycles = 2U;
        return true;

    case 0x69U:
        adc(fetch_byte());
        cycles = 2U;
        return true;

    case 0x65U:
        adc(bus_.read(addr_zero_page()));
        cycles = 3U;
        return true;

    case 0x75U:
        adc(bus_.read(addr_zero_page_x()));
        cycles = 4U;
        return true;

    case 0x6DU:
        adc(bus_.read(addr_absolute()));
        cycles = 4U;
        return true;

    case 0x7DU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        adc(bus_.read(addr));
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x79U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        adc(bus_.read(addr));
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0x61U:
        adc(bus_.read(addr_indirect_x()));
        cycles = 6U;
        return true;

    case 0x71U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        adc(bus_.read(addr));
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    case 0xE9U:
        sbc(fetch_byte());
        cycles = 2U;
        return true;

    case 0xE5U:
        sbc(bus_.read(addr_zero_page()));
        cycles = 3U;
        return true;

    case 0xF5U:
        sbc(bus_.read(addr_zero_page_x()));
        cycles = 4U;
        return true;

    case 0xEDU:
        sbc(bus_.read(addr_absolute()));
        cycles = 4U;
        return true;

    case 0xFDU: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + x_);
        sbc(bus_.read(addr));
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xF9U: {
        const u16 base = fetch_word();
        const u16 addr = static_cast<u16>(base + y_);
        sbc(bus_.read(addr));
        cycles = page_crossed(base, addr) ? 5U : 4U;
        return true;
    }

    case 0xE1U:
        sbc(bus_.read(addr_indirect_x()));
        cycles = 6U;
        return true;

    case 0xF1U: {
        const u8 zp = fetch_byte();
        const u16 base = read_word_zero_page(zp);
        const u16 addr = static_cast<u16>(base + y_);
        sbc(bus_.read(addr));
        cycles = page_crossed(base, addr) ? 6U : 5U;
        return true;
    }

    default:
        return false;
    }
}

} // namespace nes