#pragma once

#include <array>
#include <vector>

#include "mappers/mapper.hpp"
#include "rom.hpp"

namespace nes {

class Mapper23 final : public Mapper {
public:
    explicit Mapper23(Rom rom);

    u8 cpu_read(u16 addr) const override;
    void cpu_write(u16 addr, u8 value) override;

    u8 ppu_read(u16 addr) const override;
    void ppu_write(u16 addr, u8 value) override;

    void tick_cpu(usize cpu_cycles) override;
    [[nodiscard]] bool irq_pending() const noexcept override;
    void clear_irq() noexcept override;

    [[nodiscard]] bool has_dynamic_mirroring() const noexcept override;
    [[nodiscard]] Mirroring mirroring() const noexcept override;

private:
    enum class AddressVariant : u8 {
        A0A1,
        A2A3
    };

    [[nodiscard]] usize prg_offset(usize bank, usize addr_offset) const noexcept;
    [[nodiscard]] usize chr_offset(usize bank, usize addr_offset) const noexcept;

    [[nodiscard]] u8 register_index(u16 addr) const noexcept;
    [[nodiscard]] u8 register_index_a0_a1(u16 addr) const noexcept;
    [[nodiscard]] u8 register_index_a2_a3(u16 addr) const noexcept;

    void write_chr_register(u16 group, u8 reg, u8 value) noexcept;
    void write_mirroring(u8 value) noexcept;
    void write_control(u8 reg, u8 value) noexcept;
    void write_irq_register(u8 reg, u8 value) noexcept;
    void reload_irq_counter() noexcept;
    void clock_irq_counter() noexcept;

    [[nodiscard]] u8 bank_at_8000() const noexcept;
    [[nodiscard]] u8 bank_at_c000() const noexcept;

    std::vector<u8> prg_rom_;
    std::vector<u8> chr_data_;
    bool chr_is_ram_ {false};

    usize prg_bank_count_ {0};
    usize chr_bank_count_ {0};

    u8 prg_bank_8000_ {0};
    u8 prg_bank_a000_ {1};
    bool prg_mode_swap_ {false};

    std::array<u16, 8> chr_banks_ {};

    Mirroring mirroring_ {Mirroring::Horizontal};
    AddressVariant address_variant_ {AddressVariant::A0A1};

    u8 vrc2_latch_ {0};

    u8 irq_latch_ {0};
    u8 irq_counter_ {0};
    bool irq_enabled_ {false};
    bool irq_enabled_after_ack_ {false};
    bool irq_cycle_mode_ {false};
    bool irq_pending_ {false};
    int irq_prescaler_ {0};
};

} // namespace nes