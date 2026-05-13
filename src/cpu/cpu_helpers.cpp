#include "cpu.hpp"

namespace nes {

bool Cpu::get_flag(StatusFlag flag) const noexcept {
    return (status_ & flag) != 0U;
}

void Cpu::set_flag(StatusFlag flag, bool value) noexcept {
    if (value) {
        status_ = static_cast<u8>(status_ | flag);
    } else {
        status_ = static_cast<u8>(status_ & ~flag);
    }
    status_ = static_cast<u8>(status_ | Unused);
}

u8 Cpu::fetch_byte() {
    const u8 value = bus_.read(pc_);
    pc_ = static_cast<u16>(pc_ + 1U);
    return value;
}

u16 Cpu::fetch_word() {
    const u8 lo = fetch_byte();
    const u8 hi = fetch_byte();
    return static_cast<u16>((static_cast<u16>(hi) << 8U) | lo);
}

void Cpu::set_zn(u8 value) {
    set_flag(Zero, value == 0U);
    set_flag(Negative, (value & 0x80U) != 0U);
}

void Cpu::push(u8 value) {
    const u16 addr = static_cast<u16>(0x0100U | sp_);
    bus_.write(addr, value);
    sp_ = static_cast<u8>(sp_ - 1U);
}

u8 Cpu::pop() {
    sp_ = static_cast<u8>(sp_ + 1U);
    const u16 addr = static_cast<u16>(0x0100U | sp_);
    return bus_.read(addr);
}

void Cpu::push_word(u16 value) {
    push(static_cast<u8>((value >> 8U) & 0x00FFU));
    push(static_cast<u8>(value & 0x00FFU));
}

u16 Cpu::pop_word() {
    const u8 lo = pop();
    const u8 hi = pop();
    return static_cast<u16>((static_cast<u16>(hi) << 8U) | lo);
}

u16 Cpu::read_vector(u16 addr) {
    const u8 lo = bus_.read(addr);
    const u8 hi = bus_.read(static_cast<u16>(addr + 1U));
    return static_cast<u16>((static_cast<u16>(hi) << 8U) | lo);
}

void Cpu::enter_interrupt(u16 vector_addr, bool set_break_flag) {
    push_word(pc_);

    u8 pushed_status = static_cast<u8>(status_ | Unused);
    if (set_break_flag) {
        pushed_status = static_cast<u8>(pushed_status | Break);
    } else {
        pushed_status = static_cast<u8>(pushed_status & ~Break);
    }

    push(pushed_status);
    set_flag(InterruptDisable, true);
    pc_ = read_vector(vector_addr);
}

bool Cpu::page_crossed(u16 base, u16 effective) const noexcept {
    return (base & 0xFF00U) != (effective & 0xFF00U);
}

void Cpu::compare(u8 reg, u8 value) {
    const u8 result = static_cast<u8>(reg - value);
    set_flag(Carry, reg >= value);
    set_flag(Zero, reg == value);
    set_flag(Negative, (result & 0x80U) != 0U);
}

void Cpu::adc(u8 value) {
    const u16 carry_in = get_flag(Carry) ? 1U : 0U;
    const u16 sum = static_cast<u16>(a_) + static_cast<u16>(value) + carry_in;
    const u8 result = static_cast<u8>(sum & 0x00FFU);

    set_flag(Carry, sum > 0x00FFU);
    set_flag(
        Overflow,
        ((~static_cast<u8>(a_ ^ value) & static_cast<u8>(a_ ^ result)) & 0x80U) != 0U
    );

    a_ = result;
    set_zn(a_);
}

void Cpu::sbc(u8 value) {
    adc(static_cast<u8>(value ^ 0xFFU));
}

u8 Cpu::asl_value(u8 value) {
    set_flag(Carry, (value & 0x80U) != 0U);
    value = static_cast<u8>(value << 1U);
    set_zn(value);
    return value;
}

u8 Cpu::lsr_value(u8 value) {
    set_flag(Carry, (value & 0x01U) != 0U);
    value = static_cast<u8>(value >> 1U);
    set_zn(value);
    return value;
}

u8 Cpu::rol_value(u8 value) {
    const bool carry_in = get_flag(Carry);
    const bool carry_out = (value & 0x80U) != 0U;

    value = static_cast<u8>((value << 1U) | (carry_in ? 1U : 0U));

    set_flag(Carry, carry_out);
    set_zn(value);
    return value;
}

u8 Cpu::ror_value(u8 value) {
    const bool carry_in = get_flag(Carry);
    const bool carry_out = (value & 0x01U) != 0U;

    value = static_cast<u8>((value >> 1U) | (carry_in ? 0x80U : 0U));

    set_flag(Carry, carry_out);
    set_zn(value);
    return value;
}

} // namespace nes