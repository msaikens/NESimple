#include "test_cpu_helpers.hpp"

#include <stdexcept>
#include <utility>

namespace nes::test_helpers {

namespace {

CpuTestRig make_cpu_from_rom(Rom rom) {
    return CpuTestRig(std::move(rom));
}

CpuTestRig make_cpu_with_program_impl(
    std::initializer_list<u8> program,
    u16 start_addr,
    usize prg_size
) {
    if (prg_size != 16U * 1024U && prg_size != 32U * 1024U) {
        throw std::runtime_error("unsupported PRG size for test helper");
    }

    if (start_addr < 0x8000U) {
        throw std::runtime_error("test program start address must be in cartridge space");
    }

    const usize offset = static_cast<usize>(start_addr - 0x8000U);

    if (offset + program.size() > prg_size) {
        throw std::runtime_error("test program does not fit in PRG ROM");
    }

    Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(prg_size, 0);
    rom.chr_data.assign(8U * 1024U, 0);
    rom.chr_is_ram = true;

    usize i = 0;
    for (const u8 byte : program) {
        rom.prg_rom[offset + i] = byte;
        ++i;
    }

    if (prg_size == 16U * 1024U) {
        rom.prg_rom[0x3FFCU] = static_cast<u8>(start_addr & 0x00FFU);
        rom.prg_rom[0x3FFDU] = static_cast<u8>((start_addr >> 8U) & 0x00FFU);
    } else {
        rom.prg_rom[0x7FFCU] = static_cast<u8>(start_addr & 0x00FFU);
        rom.prg_rom[0x7FFDU] = static_cast<u8>((start_addr >> 8U) & 0x00FFU);
    }

    return make_cpu_from_rom(std::move(rom));
}

} // namespace

CpuTestRig make_cpu_with_program_16k(std::initializer_list<u8> program, u16 start_addr) {
    if (start_addr < 0x8000U || start_addr > 0xBFFFU) {
        throw std::runtime_error(
            "16KB test helper only supports start addresses from 0x8000 to 0xBFFF"
        );
    }

    return make_cpu_with_program_impl(program, start_addr, 16U * 1024U);
}

CpuTestRig make_cpu_with_program_32k(std::initializer_list<u8> program, u16 start_addr) {
    return make_cpu_with_program_impl(program, start_addr, 32U * 1024U);
}

CpuTestRig make_cpu_with_patched_program_32k(
    std::initializer_list<u8> program,
    u16 start_addr,
    u16 reset_vector,
    u16 irq_brk_vector,
    u16 nmi_vector,
    std::initializer_list<std::pair<u16, u8>> extra_bytes
) {
    if (start_addr < 0x8000U) {
        throw std::runtime_error("test program start address must be in cartridge space");
    }

    Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(32U * 1024U, 0);
    rom.chr_data.assign(8U * 1024U, 0);
    rom.chr_is_ram = true;

    const usize offset = static_cast<usize>(start_addr - 0x8000U);
    if (offset + program.size() > rom.prg_rom.size()) {
        throw std::runtime_error("test program does not fit in 32KB PRG ROM");
    }

    usize i = 0;
    for (const u8 byte : program) {
        rom.prg_rom[offset + i] = byte;
        ++i;
    }

    for (const auto& [addr, value] : extra_bytes) {
        if (addr < 0x8000U) {
            throw std::runtime_error("extra ROM byte patch must be in cartridge space");
        }

        const usize patch_offset = static_cast<usize>(addr - 0x8000U);
        if (patch_offset >= rom.prg_rom.size()) {
            throw std::runtime_error("extra ROM byte patch is out of range");
        }

        rom.prg_rom[patch_offset] = value;
    }

    rom.prg_rom[0x7FFAU] = static_cast<u8>(nmi_vector & 0x00FFU);
    rom.prg_rom[0x7FFBU] = static_cast<u8>((nmi_vector >> 8U) & 0x00FFU);

    rom.prg_rom[0x7FFCU] = static_cast<u8>(reset_vector & 0x00FFU);
    rom.prg_rom[0x7FFDU] = static_cast<u8>((reset_vector >> 8U) & 0x00FFU);

    rom.prg_rom[0x7FFEU] = static_cast<u8>(irq_brk_vector & 0x00FFU);
    rom.prg_rom[0x7FFFU] = static_cast<u8>((irq_brk_vector >> 8U) & 0x00FFU);

    return make_cpu_from_rom(std::move(rom));
}

} // namespace nes::test_helpers