#pragma once

#include <initializer_list>
#include <utility>

#include "bus.hpp"
#include "cartridge.hpp"
#include "common/types.hpp"
#include "cpu.hpp"
#include "rom.hpp"

namespace nes::test_helpers {

struct CpuTestRig {
    Cartridge cartridge;
    Bus bus;
    Cpu cpu;

    explicit CpuTestRig(Rom rom)
        : cartridge(std::move(rom))
        , bus(std::move(cartridge))
        , cpu(bus) {
        cpu.reset();
    }
};

CpuTestRig make_cpu_with_program_16k(
    std::initializer_list<u8> program,
    u16 start_addr = 0x8000U
);

CpuTestRig make_cpu_with_program_32k(
    std::initializer_list<u8> program,
    u16 start_addr = 0x8000U
);

CpuTestRig make_cpu_with_patched_program_32k(
    std::initializer_list<u8> program,
    u16 start_addr,
    u16 reset_vector,
    u16 irq_brk_vector,
    u16 nmi_vector = 0x0000U,
    std::initializer_list<std::pair<u16, u8>> extra_bytes = {}
);

} // namespace nes::test_helpers