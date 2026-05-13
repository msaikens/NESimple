#include "mappers/mapper23.hpp"
#include "debug/debug_log.hpp"
#include <iomanip>
#include <sstream>

#include <utility>
namespace {

std::string hex_byte(nes::u8 value) {
    std::ostringstream out;
    out << "$"
        << std::uppercase
        << std::hex
        << std::setw(2)
        << std::setfill('0')
        << static_cast<int>(value);
    return out.str();
}

std::string hex_word(nes::u16 value) {
    std::ostringstream out;
    out << "$"
        << std::uppercase
        << std::hex
        << std::setw(4)
        << std::setfill('0')
        << static_cast<int>(value);
    return out.str();
}

} // namespace
namespace nes {

Mapper23::Mapper23(Rom rom)
    : prg_rom_(std::move(rom.prg_rom))
    , chr_data_(std::move(rom.chr_data))
    , chr_is_ram_(rom.chr_is_ram)
    , mirroring_(rom.mirroring) {
    if (prg_rom_.empty() || (prg_rom_.size() % (8U * 1024U)) != 0U) {
        throw NesError("Mapper23 requires PRG ROM in 8KB banks");
    }

    if (chr_data_.empty() || (chr_data_.size() % (1U * 1024U)) != 0U) {
        throw NesError("Mapper23 requires CHR data in 1KB banks");
    }

    prg_bank_count_ = prg_rom_.size() / (8U * 1024U);
    chr_bank_count_ = chr_data_.size() / (1U * 1024U);

    prg_bank_8000_ = 0;
    prg_bank_a000_ = 1;

    for (usize i = 0; i < chr_banks_.size(); ++i) {
        chr_banks_[i] = static_cast<u16>(i);
    }
}

usize Mapper23::prg_offset(usize bank, usize addr_offset) const noexcept {
    return ((bank % prg_bank_count_) * 8U * 1024U) + addr_offset;
}

usize Mapper23::chr_offset(usize bank, usize addr_offset) const noexcept {
    return ((bank % chr_bank_count_) * 1U * 1024U) + addr_offset;
}

bool Mapper23::has_dynamic_mirroring() const noexcept {
    return true;
}

Mirroring Mapper23::mirroring() const noexcept {
    return mirroring_;
}

u8 Mapper23::register_index_a0_a1(u16 addr) const noexcept {
    return static_cast<u8>(addr & 0x0003U);
}

u8 Mapper23::register_index_a2_a3(u16 addr) const noexcept {
    return static_cast<u8>((addr >> 2U) & 0x0003U);
}

u8 Mapper23::register_index(u16 addr) const noexcept {
    if (address_variant_ == AddressVariant::A2A3) {
        return register_index_a2_a3(addr);
    }

    return register_index_a0_a1(addr);
}

u8 Mapper23::bank_at_8000() const noexcept {
    if (prg_mode_swap_) {
        return static_cast<u8>(prg_bank_count_ >= 2U ? prg_bank_count_ - 2U : 0U);
    }

    return prg_bank_8000_;
}

u8 Mapper23::bank_at_c000() const noexcept {
    if (prg_mode_swap_) {
        return prg_bank_8000_;
    }

    return static_cast<u8>(prg_bank_count_ >= 2U ? prg_bank_count_ - 2U : 0U);
}

u8 Mapper23::cpu_read(u16 addr) const {
    if (addr >= 0x6000U && addr < 0x8000U) {
        return static_cast<u8>(0x60U | (vrc2_latch_ & 0x01U));
    }

    if (addr < 0x8000U) {
        return 0;
    }

    if (addr < 0xA000U) {
        return prg_rom_[prg_offset(bank_at_8000(), addr - 0x8000U)];
    }

    if (addr < 0xC000U) {
        return prg_rom_[prg_offset(prg_bank_a000_, addr - 0xA000U)];
    }

    if (addr < 0xE000U) {
        return prg_rom_[prg_offset(bank_at_c000(), addr - 0xC000U)];
    }

    return prg_rom_[prg_offset(prg_bank_count_ - 1U, addr - 0xE000U)];
}

void Mapper23::write_mirroring(u8 value) noexcept {
    // H/V only for now. Your test suggested one-screen handling was risky here.
    switch (value & 0x03U) {
    case 0x00U:
    case 0x02U:
        mirroring_ = Mirroring::Vertical;
        break;

    case 0x01U:
    case 0x03U:
        mirroring_ = Mirroring::Horizontal;
        break;

    default:
        break;
    }
}

void Mapper23::write_control(u8 reg, u8 value) noexcept {
    if (reg == 0U) {
        write_mirroring(value);
        return;
    }

    if (reg == 2U) {
        prg_mode_swap_ = (value & 0x02U) != 0U;
    }
}

void Mapper23::write_chr_register(u16 group, u8 reg, u8 value) noexcept {
    const usize group_index = static_cast<usize>((group - 0xB000U) / 0x1000U);
    const usize pair_index = static_cast<usize>((reg >> 1U) & 0x01U);
    const usize chr_index = group_index * 2U + pair_index;

    if (chr_index >= chr_banks_.size()) {
        return;
    }

    if ((reg & 0x01U) == 0U) {
        chr_banks_[chr_index] =
            static_cast<u16>((chr_banks_[chr_index] & 0x01F0U) | (value & 0x0FU));
    } else {
        chr_banks_[chr_index] =
            static_cast<u16>((chr_banks_[chr_index] & 0x000FU) | ((value & 0x1FU) << 4U));
    }
}

void Mapper23::write_irq_register(u8 reg, u8 value) noexcept {
    DebugLog::write_if(DebugCategory::Mapper, [&] {
        std::ostringstream out;
        out << "Mapper23 IRQ write"
            << " reg=" << static_cast<int>(reg)
            << " value=" << hex_byte(value)
            << " latch_before=" << hex_byte(irq_latch_)
            << " counter_before=" << hex_byte(irq_counter_)
            << " enabled_before=" << irq_enabled_
            << " after_ack_before=" << irq_enabled_after_ack_
            << " cycle_mode_before=" << irq_cycle_mode_
            << " pending_before=" << irq_pending_;
        return out.str();
    });

    switch (reg) {
    case 0U:
        irq_latch_ = static_cast<u8>((irq_latch_ & 0xF0U) | (value & 0x0FU));
        break;

    case 1U:
        irq_latch_ = static_cast<u8>((irq_latch_ & 0x0FU) | ((value & 0x0FU) << 4U));
        break;

    case 2U:
        irq_enabled_after_ack_ = (value & 0x01U) != 0U;
        irq_enabled_ = (value & 0x02U) != 0U;
        irq_cycle_mode_ = (value & 0x04U) != 0U;
        irq_pending_ = false;
        irq_prescaler_ = 0;

        if (irq_enabled_) {
            reload_irq_counter();
        }
        break;

    case 3U:
        irq_pending_ = false;
        irq_enabled_ = irq_enabled_after_ack_;

        if (irq_enabled_) {
            reload_irq_counter();
            irq_prescaler_ = 0;
        }
        break;

    default:
        break;
    }

    DebugLog::write_if(DebugCategory::Mapper, [&] {
        std::ostringstream out;
        out << "Mapper23 IRQ state"
            << " reg=" << static_cast<int>(reg)
            << " latch_after=" << hex_byte(irq_latch_)
            << " counter_after=" << hex_byte(irq_counter_)
            << " enabled_after=" << irq_enabled_
            << " after_ack_after=" << irq_enabled_after_ack_
            << " cycle_mode_after=" << irq_cycle_mode_
            << " pending_after=" << irq_pending_;
        return out.str();
    });
}

void Mapper23::cpu_write(u16 addr, u8 value) {
    if (addr >= 0x6000U && addr < 0x8000U) {
        vrc2_latch_ = static_cast<u8>(value & 0x01U);
        return;
    }

    if (addr < 0x8000U) {
        return;
    }

    const u16 group = static_cast<u16>(addr & 0xF000U);
    const u8 reg = register_index(addr);
DebugLog::write_if(DebugCategory::Mapper, [&] {
    std::ostringstream out;
    out << "Mapper23 write"
        << " addr=" << hex_word(addr)
        << " group=" << hex_word(group)
        << " reg=" << static_cast<int>(reg)
        << " value=" << hex_byte(value)
        << " prg8000=" << static_cast<int>(prg_bank_8000_)
        << " prgA000=" << static_cast<int>(prg_bank_a000_)
        << " prgSwap=" << prg_mode_swap_
        << " mirroring=" << static_cast<int>(mirroring_);
    return out.str();
});
    switch (group) {
    case 0x8000U:
        prg_bank_8000_ = static_cast<u8>(value & 0x1FU);
        break;

    case 0x9000U:
        write_control(reg, value);
        break;

    case 0xA000U:
        prg_bank_a000_ = static_cast<u8>(value & 0x1FU);
        break;

    case 0xB000U:
    case 0xC000U:
    case 0xD000U:
    case 0xE000U:
        write_chr_register(group, reg, value);
        break;

    case 0xF000U:
        write_irq_register(reg, value);
        break;

    default:
        break;
    }
}
void Mapper23::reload_irq_counter() noexcept {
    irq_counter_ = static_cast<u8>(irq_latch_ - 1U);
}
void Mapper23::clock_irq_counter() noexcept {
    if (irq_counter_ == 0xFFU) {
        reload_irq_counter();
        irq_pending_ = true;

        DebugLog::write_if(DebugCategory::Mapper, [&] {
            std::ostringstream out;
            out << "Mapper23 IRQ FIRED"
                << " latch=" << hex_byte(irq_latch_)
                << " counter=" << hex_byte(irq_counter_)
                << " cycle_mode=" << irq_cycle_mode_
                << " prescaler=" << irq_prescaler_;
            return out.str();
        });

        return;
    }

    ++irq_counter_;
}

void Mapper23::tick_cpu(usize cpu_cycles) {
    if (!irq_enabled_) {
        return;
    }

    if (irq_cycle_mode_) {
        for (usize i = 0; i < cpu_cycles; ++i) {
            clock_irq_counter();
        }
        return;
    }

    // Approximate VRC scanline IRQ mode:
    // count one IRQ tick per ~341 PPU cycles, or ~113.667 CPU cycles.
    irq_prescaler_ += static_cast<int>(cpu_cycles) * 3;

while (irq_prescaler_ >= 341) {
    irq_prescaler_ -= 341;
    clock_irq_counter();
}
}

bool Mapper23::irq_pending() const noexcept {
    return irq_pending_;
}

void Mapper23::clear_irq() noexcept {
    irq_pending_ = false;
}

u8 Mapper23::ppu_read(u16 addr) const {
    if (addr >= 0x2000U) {
        return 0;
    }

    const usize bank_index = static_cast<usize>(addr / 0x0400U);
    const usize offset = static_cast<usize>(addr & 0x03FFU);

    return chr_data_[chr_offset(chr_banks_[bank_index], offset)];
}

void Mapper23::ppu_write(u16 addr, u8 value) {
    if (addr >= 0x2000U) {
        return;
    }

    if (!chr_is_ram_) {
        return;
    }

    const usize bank_index = static_cast<usize>(addr / 0x0400U);
    const usize offset = static_cast<usize>(addr & 0x03FFU);

    chr_data_[chr_offset(chr_banks_[bank_index], offset)] = value;
}

} // namespace nes