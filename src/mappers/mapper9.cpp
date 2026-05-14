#include "mappers/mapper9.hpp"

#include <utility>

namespace nes {

Mapper9::Mapper9(Rom rom)
    : prg_rom_(std::move(rom.prg_rom))
    , chr_data_(std::move(rom.chr_data))
    , chr_is_ram_(rom.chr_is_ram)
    , mirroring_(rom.mirroring) {
    if (prg_rom_.empty() || (prg_rom_.size() % (8U * 1024U)) != 0U) {
        throw NesError("Mapper9 requires PRG ROM in 8KB banks");
    }

    if (chr_data_.empty() || (chr_data_.size() % (4U * 1024U)) != 0U) {
        throw NesError("Mapper9 requires CHR data in 4KB banks");
    }

    prg_bank_count_ = prg_rom_.size() / (8U * 1024U);
    chr_bank_count_ = chr_data_.size() / (4U * 1024U);

    prg_bank_8000_ = 0;

    chr_fd_0000_ = 0;
    chr_fe_0000_ = 0;
    chr_fd_1000_ = 0;
    chr_fe_1000_ = 0;

    latch_0000_ = LatchState::FD;
    latch_1000_ = LatchState::FD;
}

usize Mapper9::prg_offset(usize bank, usize addr_offset) const noexcept {
    return ((bank % prg_bank_count_) * 8U * 1024U) + addr_offset;
}

usize Mapper9::chr_offset(usize bank, usize addr_offset) const noexcept {
    return ((bank % chr_bank_count_) * 4U * 1024U) + addr_offset;
}

bool Mapper9::has_dynamic_mirroring() const noexcept {
    return true;
}

Mirroring Mapper9::mirroring() const noexcept {
    return mirroring_;
}

u8 Mapper9::cpu_read(u16 addr) const {
    if (addr >= 0x6000U && addr < 0x8000U) {
        return prg_ram_[addr - 0x6000U];
    }

    if (addr < 0x8000U) {
        return 0;
    }

    if (addr < 0xA000U) {
        return prg_rom_[prg_offset(prg_bank_8000_, addr - 0x8000U)];
    }

    if (addr < 0xC000U) {
        return prg_rom_[prg_offset(prg_bank_count_ - 3U, addr - 0xA000U)];
    }

    if (addr < 0xE000U) {
        return prg_rom_[prg_offset(prg_bank_count_ - 2U, addr - 0xC000U)];
    }

    return prg_rom_[prg_offset(prg_bank_count_ - 1U, addr - 0xE000U)];
}

void Mapper9::cpu_write(u16 addr, u8 value) {
    if (addr >= 0x6000U && addr < 0x8000U) {
        prg_ram_[addr - 0x6000U] = value;
        return;
    }

    if (addr < 0xA000U) {
        return;
    }

    switch (addr & 0xF000U) {
    case 0xA000U:
        prg_bank_8000_ = static_cast<u8>(value & 0x0FU);
        break;

    case 0xB000U:
        chr_fd_0000_ = static_cast<u8>(value & 0x1FU);
        break;

    case 0xC000U:
        chr_fe_0000_ = static_cast<u8>(value & 0x1FU);
        break;

    case 0xD000U:
        chr_fd_1000_ = static_cast<u8>(value & 0x1FU);
        break;

    case 0xE000U:
        chr_fe_1000_ = static_cast<u8>(value & 0x1FU);
        break;

    case 0xF000U:
        mirroring_ = (value & 0x01U) != 0U
            ? Mirroring::Horizontal
            : Mirroring::Vertical;
        break;

    default:
        break;
    }
}

void Mapper9::update_latches_for_ppu_read(u16 addr) const noexcept {
    addr &= 0x1FFFU;

    // MMC2 latch behavior:
    // $0FD8 -> latch 0 = FD
    // $0FE8 -> latch 0 = FE
    // $1FD8-$1FDF -> latch 1 = FD
    // $1FE8-$1FEF -> latch 1 = FE
    //
    // Punch-Out relies on these PPU read side effects. NESdev notes latch 0
    // responds to one address while latch 1 responds to a range.
    if (addr == 0x0FD8U) {
        latch_0000_ = LatchState::FD;
    } else if (addr == 0x0FE8U) {
        latch_0000_ = LatchState::FE;
    } else if (addr >= 0x1FD8U && addr <= 0x1FDFU) {
        latch_1000_ = LatchState::FD;
    } else if (addr >= 0x1FE8U && addr <= 0x1FEFU) {
        latch_1000_ = LatchState::FE;
    }
}

usize Mapper9::selected_chr_bank_for_addr(u16 addr) const noexcept {
    if (addr < 0x1000U) {
        return latch_0000_ == LatchState::FD
            ? static_cast<usize>(chr_fd_0000_)
            : static_cast<usize>(chr_fe_0000_);
    }

    return latch_1000_ == LatchState::FD
        ? static_cast<usize>(chr_fd_1000_)
        : static_cast<usize>(chr_fe_1000_);
}

u8 Mapper9::ppu_read(u16 addr) const {
    if (addr >= 0x2000U) {
        return 0;
    }

    addr &= 0x1FFFU;

    const usize bank = selected_chr_bank_for_addr(addr);
    const usize offset = static_cast<usize>(addr & 0x0FFFU);
    const u8 value = chr_data_[chr_offset(bank, offset)];

    // MMC2 latch changes affect subsequent reads, not the current byte.
    update_latches_for_ppu_read(addr);

    return value;
}

void Mapper9::ppu_write(u16 addr, u8 value) {
    if (addr >= 0x2000U) {
        return;
    }

    if (!chr_is_ram_) {
        return;
    }

    addr &= 0x1FFFU;

    const usize bank = selected_chr_bank_for_addr(addr);
    const usize offset = static_cast<usize>(addr & 0x0FFFU);

    chr_data_[chr_offset(bank, offset)] = value;

    update_latches_for_ppu_read(addr);
}

} // namespace nes