#include "mappers/mapper0.hpp"

namespace nes {
// Mapper0 implements the simplest NES cartridge configuration, known as NROM. It supports either 16 KB or 32 KB of PRG ROM and 8 KB of CHR data.
// The PRG ROM is mapped into CPU address space starting at $8000, with
// 16 KB cartridges mirrored into both $8000-$BFFF and $C000-$FFFF. The CHR data is mapped into PPU address space at $0000-$1FFF.
// The constructor takes a Rom object as input, which contains all the necessary data and metadata for the cartridge. It validates that the Rom
// contains the expected PRG and CHR sizes for NROM, and initializes the internal state
// accordingly. If the Rom does not meet the requirements for Mapper 0 (e.g. incorrect PRG or CHR sizes), we throw a NesError to indicate 
// the problem. By centralizing all validation and initialization logic in the constructor, we ensure that any Mapper0 instance is 
//properly configured and ready to use.
Mapper0::Mapper0(Rom rom)
    : prg_rom_(std::move(rom.prg_rom)),
      chr_data_(std::move(rom.chr_data)),
      chr_is_ram_(rom.chr_is_ram) {
    if (!(prg_rom_.size() == 16U * 1024U || prg_rom_.size() == 32U * 1024U)) {
        throw NesError("Mapper0 requires 16KB or 32KB PRG ROM");
    }
// Mapper 0 cartridges always have 8 KB of CHR data, which may be ROM or RAM. We validate that the CHR data is the correct size,
// and we set the chr_is_ram_ flag based on the input Rom. This allows us to support both types of cartridges while keeping the logic
// for handling CHR data cleanly encapsulated within the Mapper0 class. 
// By performing this validation in the constructor, we ensure that any Mapper0 instance is properly configured and that the rest of the emulator
    if (chr_data_.size() != 8U * 1024U) {
        throw NesError("Mapper0 expects 8KB CHR data");
    }
}
// The cpu_read method implements the logic for how CPU addresses map to the underlying PRG ROM. For Mapper 0, 
// the PRG ROM is mapped starting at $8000, and if the cartridge has only 16 KB of PRG ROM, it is mirrored into both $8000-$BFFF and $C000-$FFFF.
// By implementing this logic in the cpu_read method, we allow the rest of the emulator to
// interact with the cartridge through a consistent interface, while the specific implementation details are 
// encapsulated within the Mapper0 class.
u8 Mapper0::cpu_read(u16 addr) const {
    if (addr < 0x8000U) {
        return 0;
    }
// Calculate the offset into the PRG ROM based on the input address. If the cartridge has only 16 KB of PRG ROM, 
// we mirror it by taking the offset modulo 16 KB.
    usize offset = static_cast<usize>(addr - 0x8000U);

    if (prg_rom_.size() == 16U * 1024U) {
        offset %= 16U * 1024U;
    }

    return prg_rom_[offset];
}
// The cpu_write method for Mapper 0 does not allow any writes to the PRG ROM, as it is read-only. However, we still need to implement this method
// to satisfy the Mapper interface. In this implementation, we simply ignore any writes to the PRG 
// address range. By providing a no-op implementation, we ensure that the rest of the emulator can call cpu_write without 
// needing to worry about the specific behavior of Mapper 0, while still maintaining the integrity of the PRG ROM data.
void Mapper0::cpu_write(u16 /*addr*/, u8 /*value*/) {
}
// The ppu_read and ppu_write methods implement the logic for how PPU addresses map to the underlying CHR data. For Mapper 0,
// the CHR data is mapped starting at $0000, and it may be either ROM (read-only) or RAM (writable) based on the chr_is_ram_ flag.
// By implementing this logic in the ppu_read and ppu_write methods, we allow the
// rest of the emulator to interact with the cartridge's CHR data through a consistent interface, while the specific implementation details are
// encapsulated within the Mapper0 class. This design allows us to support both types of cartridges
// while keeping the logic for handling CHR data cleanly organized within the Mapper0 implementation.
u8 Mapper0::ppu_read(u16 addr) const {
    if (addr >= 0x2000U) {
        return 0;
    }

    return chr_data_[addr];
}
// The ppu_write method checks if the address is within the CHR data range ($0000-$1FFF) and if the CHR is RAM. If both conditions are true,
// it allows the write to modify the CHR data. If the CHR is ROM or if the address is out of range, the write is ignored. This ensures that we
// maintain the integrity of the CHR data based on the cartridge configuration, while still allowing for
// writable CHR when appropriate. By implementing this logic in the ppu_write method, we allow the rest of the emulator 
// to interact with the cartridge's CHR data
void Mapper0::ppu_write(u16 addr, u8 value) {
    if (addr >= 0x2000U) {
        return;
    }
// Only allow writes if CHR is RAM. If CHR is ROM, we ignore the write to maintain data integrity.
// By checking the chr_is_ram_ flag, we can support both types of cartridges while keeping the logic for handling 
// CHR data cleanly encapsulated within the Mapper0 class.
    if (chr_is_ram_) {
        chr_data_[addr] = value;
    }
}

}