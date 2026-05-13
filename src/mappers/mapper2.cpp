#include "mappers/mapper2.hpp"

#include <iostream>
#include <utility>

namespace nes {

Mapper2::Mapper2(Rom rom)
    : prg_rom_(std::move(rom.prg_rom)),
      chr_data_(std::move(rom.chr_data)),
      chr_is_ram_(rom.chr_is_ram) {
    if (prg_rom_.empty() || (prg_rom_.size() % (16U * 1024U)) != 0U) {
        throw NesError("Mapper2 requires PRG ROM in 16KB banks");
    }

    if (chr_data_.size() != 8U * 1024U) {
        throw NesError("Mapper2 expects 8KB CHR data");
    }

    prg_bank_count_ = prg_rom_.size() / (16U * 1024U);
    selected_bank_ = 0;
}

u8 Mapper2::cpu_read(u16 addr) const {
    if (addr >= 0x6000U && addr < 0x8000U) {
        return prg_ram_[addr - 0x6000U];
    }

    if (addr < 0x8000U) {
        return 0;
    }

    if (addr < 0xC000U) {
        const usize bank = selected_bank_ % prg_bank_count_;
        const usize offset = bank * 0x4000U + static_cast<usize>(addr - 0x8000U);
        return prg_rom_[offset];
    }

    const usize fixed_bank = prg_bank_count_ - 1U;
    const usize offset = fixed_bank * 0x4000U + static_cast<usize>(addr - 0xC000U);
    return prg_rom_[offset];
}

void Mapper2::cpu_write(u16 addr, u8 value) {
    if (addr >= 0x6000U && addr < 0x8000U) {
        prg_ram_[addr - 0x6000U] = value;
        return;
    }

    if (addr >= 0x8000U) {
        const usize old_bank = selected_bank_;
        selected_bank_ = static_cast<usize>(value) % prg_bank_count_;

//        if (old_bank != selected_bank_) {
//            std::cout
//                << "MAPPER2 BANK SWITCH "
//                << "addr=0x" << std::hex << static_cast<int>(addr)
//                << " value=0x" << static_cast<int>(value)
//                << " old=" << std::dec << old_bank
//                << " new=" << selected_bank_
//                << " bank_count=" << prg_bank_count_
//                << '\n';
//        }
    }
}

u8 Mapper2::ppu_read(u16 addr) const {
    if (addr >= 0x2000U) {
        return 0;
    }

    return chr_data_[addr];
}

void Mapper2::ppu_write(u16 addr, u8 value) {
    if (addr >= 0x2000U) {
        return;
    }

    if (chr_is_ram_) {
        chr_data_[addr] = value;
    }
}

} // namespace nes