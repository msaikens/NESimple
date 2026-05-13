#pragma once

#include "mapper.hpp"
#include "rom.hpp"

namespace nes {
// Mapper0 implements the simplest NES cartridge configuration, known as NROM. It supports either 16 KB or 32 KB of PRG ROM and 8 KB of CHR data.
// The PRG ROM is mapped into CPU address space starting at $8000, with
// 16 KB cartridges mirrored into both $8000-$BFFF and $C000-$FFFF. The CHR data is mapped into PPU address space at $0000-$1FFF.
class Mapper0 final : public Mapper {
public:
// Construct a Mapper0 instance from a parsed Rom. This constructor validates that the Rom contains the expected PRG and CHR sizes for NROM,
// and initializes the internal state accordingly. By taking a Rom object as input, we can ensure that all necessary cartridge data is available
// and that the mapper is properly configured based on the ROM's metadata.
// If the Rom does not meet the requirements for Mapper 0 (e.g. incorrect PRG or CHR sizes), we throw a NesError to indicate the problem.
    explicit Mapper0(Rom rom);
// Implement the Mapper interface for CPU and PPU reads and writes. The logic for how addresses map to the underlying PRG ROM and CHR data is 
// defined here. By implementing these methods, we allow the rest of the emulator to interact with the cartridge through a consistent interface, 
// while the specific implementation details are encapsulated within the Mapper0 class.
    u8 cpu_read(u16 addr) const override;
    void cpu_write(u16 addr, u8 value) override;
    u8 ppu_read(u16 addr) const override;
    void ppu_write(u16 addr, u8 value) override;
// The internal state of the Mapper0 includes the PRG ROM data, CHR data, and a flag indicating whether CHR is RAM. 
// This state is initialized in the constructor based on the input Rom and is used in the read/write methods to determine 
// how to respond to CPU and PPU accesses.
private:
// The PRG ROM and CHR data are stored as vectors of bytes. The chr_is_ram_ flag indicates whether the CHR data should be 
// treated as RAM (writable) or ROM (read-only).
    std::vector<u8> prg_rom_;
    std::vector<u8> chr_data_;
    bool chr_is_ram_ {false};
};

}