#include "cpu.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>
namespace nes {

Cpu::Cpu(Bus& bus)
    : bus_(bus) {
}

void Cpu::reset() {
    pc_ = read_vector(0xFFFCU);
    sp_ = 0xFDU;
    status_ = static_cast<u8>(InterruptDisable | Unused);
}

void Cpu::set_pc(u16 value) noexcept {
    pc_ = value;
}

void Cpu::irq() {
    if (get_flag(InterruptDisable)) {
        return;
    }

    enter_interrupt(0xFFFEU, false);
}

void Cpu::nmi() {
    if (in_nmi_) {
        return;
    }

    in_nmi_ = true;

//    std::cout
//        << "ENTER NMI PC="
//        << std::hex
//        << pc_
//        << '\n';

    enter_interrupt(0xFFFAU, false);
}

usize Cpu::step() {
    const u16 pc_before = pc_;
    const u8 opcode = fetch_byte();

    if (logging_enabled_ && log_stream_ != nullptr) {
        log_instruction(pc_before, opcode);
    }

    usize cycles = 0;

    if (handle_load_store(opcode, cycles)
        || handle_logic(opcode, cycles)
        || handle_flow(opcode, cycles)
        || handle_stack(opcode, cycles)
        || handle_shift(opcode, cycles)) {
            if (cycles == 0) {
//    std::cout
//        << "ZERO CYCLE OPCODE "
//        << std::hex
//        << static_cast<int>(opcode)
//        << " PC="
//        << pc_before
//       << '\n';
}
        bus_.tick_cpu(cycles);

        if (bus_.poll_nmi()) {
            nmi();
        } else if (bus_.poll_irq()) {
            irq();
        }

        return cycles;
    }

    std::ostringstream out;
    out << "Unimplemented opcode $"
        << std::uppercase << std::hex
        << std::setw(2) << std::setfill('0')
        << static_cast<int>(opcode)
        << " at PC $"
        << std::setw(4)
        << static_cast<int>(pc_before)
        << " next PC $"
        << std::setw(4)
        << static_cast<int>(pc_)
        << " A:$"
        << std::setw(2)
        << static_cast<int>(a_)
        << " X:$"
        << std::setw(2)
        << static_cast<int>(x_)
        << " Y:$"
        << std::setw(2)
        << static_cast<int>(y_)
        << " P:$"
        << std::setw(2)
        << static_cast<int>(status_)
        << " SP:$"
        << std::setw(2)
        << static_cast<int>(sp_);

    throw NesError(out.str());
}

u8 Cpu::a() const noexcept {
    return a_;
}

u8 Cpu::x() const noexcept {
    return x_;
}

u8 Cpu::y() const noexcept {
    return y_;
}

u8 Cpu::sp() const noexcept {
    return sp_;
}

u8 Cpu::status() const noexcept {
    return status_;
}

u16 Cpu::pc() const noexcept {
    return pc_;
}

Bus& Cpu::bus() noexcept {
    return bus_;
}

const Bus& Cpu::bus() const noexcept {
    return bus_;
}

} // namespace nes