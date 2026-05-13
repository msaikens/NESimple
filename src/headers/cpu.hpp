#pragma once

#include <iosfwd>
#include <string>

#include "bus.hpp"
#include "common/types.hpp"

namespace nes {

class Cpu {
public:
    enum class TraceFormat {
        Compact,
        NestestLike
    };

    explicit Cpu(Bus& bus);

    void reset();
    void set_pc(u16 value) noexcept;
    void irq();
    void nmi();
    usize step();

    void set_logging(
        bool enabled,
        std::ostream* stream = nullptr,
        TraceFormat format = TraceFormat::Compact
    ) noexcept;

    [[nodiscard]] std::string trace_line();
    [[nodiscard]] std::string trace_line(TraceFormat format);

    [[nodiscard]] u8 a() const noexcept;
    [[nodiscard]] u8 x() const noexcept;
    [[nodiscard]] u8 y() const noexcept;
    [[nodiscard]] u8 sp() const noexcept;
    [[nodiscard]] u8 status() const noexcept;
    [[nodiscard]] u16 pc() const noexcept;

    Bus& bus() noexcept;
    const Bus& bus() const noexcept;

private:
    enum StatusFlag : u8 {
        Carry = 1U << 0U,
        Zero = 1U << 1U,
        InterruptDisable = 1U << 2U,
        Decimal = 1U << 3U,
        Break = 1U << 4U,
        Unused = 1U << 5U,
        Overflow = 1U << 6U,
        Negative = 1U << 7U
    };

    [[nodiscard]] bool get_flag(StatusFlag flag) const noexcept;
    void set_flag(StatusFlag flag, bool value) noexcept;

    u8 fetch_byte();
    u16 fetch_word();
    void set_zn(u8 value);

    void push(u8 value);
    u8 pop();
    void push_word(u16 value);
    u16 pop_word();

    [[nodiscard]] u16 read_vector(u16 addr);
    void enter_interrupt(u16 vector_addr, bool set_break_flag);

    usize branch_if(bool condition);

    [[nodiscard]] u16 addr_zero_page();
    [[nodiscard]] u16 addr_zero_page_x();
    [[nodiscard]] u16 addr_zero_page_y();
    [[nodiscard]] u16 addr_absolute();
    [[nodiscard]] u16 addr_absolute_x();
    [[nodiscard]] u16 addr_absolute_y();
    [[nodiscard]] u16 addr_indirect_x();
    [[nodiscard]] u16 addr_indirect_y();
    [[nodiscard]] u16 read_word_zero_page(u8 addr);
    [[nodiscard]] u16 read_word_bug(u16 addr);

    void compare(u8 reg, u8 value);
    void adc(u8 value);
    void sbc(u8 value);

    u8 asl_value(u8 value);
    u8 lsr_value(u8 value);
    u8 rol_value(u8 value);
    u8 ror_value(u8 value);

    void log_instruction(u16 pc_before, u8 opcode);
    [[nodiscard]] std::string disassemble_supported(u16 pc_before, u8 opcode);

    [[nodiscard]] bool handle_load_store(u8 opcode, usize& cycles);
    [[nodiscard]] bool handle_logic(u8 opcode, usize& cycles);
    [[nodiscard]] bool handle_flow(u8 opcode, usize& cycles);
    [[nodiscard]] bool handle_stack(u8 opcode, usize& cycles);
    [[nodiscard]] bool handle_shift(u8 opcode, usize& cycles);
    [[nodiscard]] bool page_crossed(u16 base, u16 effective) const noexcept;
    
    Bus& bus_;
    u8 a_ {0};
    u8 x_ {0};
    u8 y_ {0};
    u8 sp_ {0xFD};
    u8 status_ {static_cast<u8>(InterruptDisable | Unused)};
    u16 pc_ {0};
    bool in_nmi_ {false};
    bool logging_enabled_ {false};
    std::ostream* log_stream_ {nullptr};
    TraceFormat trace_format_ {TraceFormat::Compact};
};

} // namespace nes