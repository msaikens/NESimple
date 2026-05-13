#include "cpu.hpp"

namespace nes {

usize Cpu::branch_if(bool condition) {
    const auto offset = static_cast<std::int8_t>(fetch_byte());

    if (!condition) {
        return 2U;
    }

    const u16 old_pc = pc_;
    pc_ = static_cast<u16>(pc_ + offset);
    return page_crossed(old_pc, pc_) ? 4U : 3U;
}

u16 Cpu::addr_zero_page() {
    return fetch_byte();
}

u16 Cpu::addr_zero_page_x() {
    return static_cast<u8>(fetch_byte() + x_);
}

u16 Cpu::addr_zero_page_y() {
    return static_cast<u8>(fetch_byte() + y_);
}

u16 Cpu::addr_absolute() {
    return fetch_word();
}

u16 Cpu::addr_absolute_x() {
    return static_cast<u16>(fetch_word() + x_);
}

u16 Cpu::addr_absolute_y() {
    return static_cast<u16>(fetch_word() + y_);
}

u16 Cpu::read_word_zero_page(u8 addr) {
    const u8 lo = bus_.read(addr);
    const u8 hi = bus_.read(static_cast<u8>(addr + 1U));
    return static_cast<u16>((static_cast<u16>(hi) << 8U) | lo);
}

u16 Cpu::read_word_bug(u16 addr) {
    const u8 lo = bus_.read(addr);
    const u16 hi_addr =
        static_cast<u16>((addr & 0xFF00U) | static_cast<u8>((addr & 0x00FFU) + 1U));
    const u8 hi = bus_.read(hi_addr);
    return static_cast<u16>((static_cast<u16>(hi) << 8U) | lo);
}

u16 Cpu::addr_indirect_x() {
    const u8 base = static_cast<u8>(fetch_byte() + x_);
    return read_word_zero_page(base);
}

u16 Cpu::addr_indirect_y() {
    const u8 base = fetch_byte();
    return static_cast<u16>(read_word_zero_page(base) + y_);
}

} // namespace nes