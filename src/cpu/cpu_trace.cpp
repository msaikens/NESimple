#include "cpu.hpp"

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

namespace nes {

void Cpu::set_logging(bool enabled, std::ostream* stream, TraceFormat format) noexcept {
    logging_enabled_ = enabled;
    log_stream_ = stream;
    trace_format_ = format;
}

std::string Cpu::trace_line() {
    return trace_line(trace_format_);
}

std::string Cpu::trace_line(TraceFormat format) {
    std::ostringstream out;

    if (format == TraceFormat::Compact) {
        out << std::hex << std::uppercase << std::setfill('0')
            << "PC=" << std::setw(4) << static_cast<unsigned>(pc_)
            << " OP=" << std::setw(2) << static_cast<unsigned>(bus_.read(pc_))
            << " A=" << std::setw(2) << static_cast<unsigned>(a_)
            << " X=" << std::setw(2) << static_cast<unsigned>(x_)
            << " Y=" << std::setw(2) << static_cast<unsigned>(y_)
            << " SP=" << std::setw(2) << static_cast<unsigned>(sp_)
            << " P=" << std::setw(2) << static_cast<unsigned>(status_);
        return out.str();
    }

    const u8 opcode = bus_.read(pc_);
    out << disassemble_supported(pc_, opcode)
        << "  A:" << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
        << static_cast<unsigned>(a_)
        << " X:" << std::setw(2) << static_cast<unsigned>(x_)
        << " Y:" << std::setw(2) << static_cast<unsigned>(y_)
        << " P:" << std::setw(2) << static_cast<unsigned>(status_)
        << " SP:" << std::setw(2) << static_cast<unsigned>(sp_);
    return out.str();
}

std::string Cpu::disassemble_supported(u16 pc_before, u8 opcode) {
    auto read8 = [this](u16 addr) -> u8 { return bus_.read(addr); };

    const u8 b1 = read8(pc_before);
    const u8 b2 = read8(static_cast<u16>(pc_before + 1U));
    const u8 b3 = read8(static_cast<u16>(pc_before + 2U));
    const u16 word = static_cast<u16>((static_cast<u16>(b3) << 8U) | b2);

    std::ostringstream out;
    out << std::hex << std::uppercase << std::setfill('0')
        << std::setw(4) << static_cast<unsigned>(pc_before) << "  ";

    auto emit1 = [&]() {
        out << std::setw(2) << static_cast<unsigned>(b1) << "        ";
    };
    auto emit2 = [&]() {
        out << std::setw(2) << static_cast<unsigned>(b1) << " "
            << std::setw(2) << static_cast<unsigned>(b2) << "     ";
    };
    auto emit3 = [&]() {
        out << std::setw(2) << static_cast<unsigned>(b1) << " "
            << std::setw(2) << static_cast<unsigned>(b2) << " "
            << std::setw(2) << static_cast<unsigned>(b3) << "  ";
    };

    auto rel_target = [&]() -> u16 {
        const auto offset = static_cast<std::int8_t>(b2);
        return static_cast<u16>(pc_before + 2U + offset);
    };

    auto hex2 = [&](u8 value) {
        std::ostringstream s;
        s << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
          << static_cast<unsigned>(value);
        return s.str();
    };

    auto hex4 = [&](u16 value) {
        std::ostringstream s;
        s << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
          << static_cast<unsigned>(value);
        return s.str();
    };

    auto fmt1 = [&](const char* m) {
        emit1();
        out << m;
    };
    auto fmt2 = [&](const char* m, const std::string& op) {
        emit2();
        out << m << " " << op;
    };
    auto fmt3 = [&](const char* m, const std::string& op) {
        emit3();
        out << m << " " << op;
    };

    switch (opcode) {
    case 0x00U: fmt1("BRK"); break;
    case 0x08U: fmt1("PHP"); break;
    case 0x10U: fmt2("BPL", "$" + hex4(rel_target())); break;
    case 0x18U: fmt1("CLC"); break;
    case 0x20U: fmt3("JSR", "$" + hex4(word)); break;
    case 0x24U: fmt2("BIT", "$" + hex2(b2)); break;
    case 0x28U: fmt1("PLP"); break;
    case 0x2CU: fmt3("BIT", "$" + hex4(word)); break;
    case 0x30U: fmt2("BMI", "$" + hex4(rel_target())); break;
    case 0x38U: fmt1("SEC"); break;
    case 0x40U: fmt1("RTI"); break;
    case 0x48U: fmt1("PHA"); break;
    case 0x4CU: fmt3("JMP", "$" + hex4(word)); break;
    case 0x50U: fmt2("BVC", "$" + hex4(rel_target())); break;
    case 0x58U: fmt1("CLI"); break;
    case 0x60U: fmt1("RTS"); break;
    case 0x68U: fmt1("PLA"); break;
    case 0x69U: fmt2("ADC", "#$" + hex2(b2)); break;
    case 0x6CU: fmt3("JMP", "($" + hex4(word) + ")"); break;
    case 0x70U: fmt2("BVS", "$" + hex4(rel_target())); break;
    case 0x78U: fmt1("SEI"); break;
    case 0x88U: fmt1("DEY"); break;
    case 0x8AU: fmt1("TXA"); break;
    case 0x90U: fmt2("BCC", "$" + hex4(rel_target())); break;
    case 0x98U: fmt1("TYA"); break;
    case 0x9AU: fmt1("TXS"); break;
    case 0xA0U: fmt2("LDY", "#$" + hex2(b2)); break;
    case 0xA2U: fmt2("LDX", "#$" + hex2(b2)); break;
    case 0xA4U: fmt2("LDY", "$" + hex2(b2)); break;
    case 0xA5U: fmt2("LDA", "$" + hex2(b2)); break;
    case 0xA6U: fmt2("LDX", "$" + hex2(b2)); break;
    case 0xA8U: fmt1("TAY"); break;
    case 0xA9U: fmt2("LDA", "#$" + hex2(b2)); break;
    case 0xAAU: fmt1("TAX"); break;
    case 0xACU: fmt3("LDY", "$" + hex4(word)); break;
    case 0xADU: fmt3("LDA", "$" + hex4(word)); break;
    case 0xAEU: fmt3("LDX", "$" + hex4(word)); break;
    case 0xB0U: fmt2("BCS", "$" + hex4(rel_target())); break;
    case 0xB8U: fmt1("CLV"); break;
    case 0xBAU: fmt1("TSX"); break;
    case 0xC0U: fmt2("CPY", "#$" + hex2(b2)); break;
    case 0xC4U: fmt2("CPY", "$" + hex2(b2)); break;
    case 0xC8U: fmt1("INY"); break;
    case 0xC9U: fmt2("CMP", "#$" + hex2(b2)); break;
    case 0xCAU: fmt1("DEX"); break;
    case 0xCCU: fmt3("CPY", "$" + hex4(word)); break;
    case 0xD0U: fmt2("BNE", "$" + hex4(rel_target())); break;
    case 0xD8U: fmt1("CLD"); break;
    case 0xE0U: fmt2("CPX", "#$" + hex2(b2)); break;
    case 0xE4U: fmt2("CPX", "$" + hex2(b2)); break;
    case 0xE8U: fmt1("INX"); break;
    case 0xE9U: fmt2("SBC", "#$" + hex2(b2)); break;
    case 0xEAU: fmt1("NOP"); break;
    case 0xECU: fmt3("CPX", "$" + hex4(word)); break;
    case 0xF0U: fmt2("BEQ", "$" + hex4(rel_target())); break;
    case 0xF8U: fmt1("SED"); break;
    default:
        emit1();
        out << "OP $" << hex2(opcode);
        break;
    }

    return out.str();
}

void Cpu::log_instruction(u16 pc_before, u8 opcode) {
    if (trace_format_ == TraceFormat::Compact) {
        const u8 b1 = bus_.read(static_cast<u16>(pc_before + 1U));
        const u8 b2 = bus_.read(static_cast<u16>(pc_before + 2U));

(*log_stream_) << std::hex << std::uppercase << std::setfill('0')
               << "PC=" << std::setw(4) << static_cast<unsigned>(pc_before)
               << " OP=" << std::setw(2) << static_cast<unsigned>(opcode)
               << " BYTES="
               << std::setw(2) << static_cast<unsigned>(opcode) << ' '
               << std::setw(2) << static_cast<unsigned>(b1) << ' '
               << std::setw(2) << static_cast<unsigned>(b2)
               << " A=" << std::setw(2) << static_cast<unsigned>(a_)
               << " X=" << std::setw(2) << static_cast<unsigned>(x_)
               << " Y=" << std::setw(2) << static_cast<unsigned>(y_)
               << " SP=" << std::setw(2) << static_cast<unsigned>(sp_)
               << " P=" << std::setw(2) << static_cast<unsigned>(status_)
               << '\n';
        return;
    }

    const u16 saved_pc = pc_;
    pc_ = pc_before;
    (*log_stream_) << trace_line(TraceFormat::NestestLike) << '\n';
    pc_ = saved_pc;
}

} // namespace nes