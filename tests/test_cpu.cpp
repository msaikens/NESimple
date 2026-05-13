#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

#include "test_cpu_helpers.hpp"

using namespace nes;
using namespace nes::test_helpers;

TEST_CASE("CPU reset loads reset vector and initializes registers") {
    const auto rig = make_cpu_with_program_16k({0xEAU}, 0x8000U);
    auto& cpu = rig.cpu;

    REQUIRE(cpu.pc() == 0x8000U);
    REQUIRE(cpu.sp() == 0xFDU);
    REQUIRE((cpu.status() & 0x24U) == 0x24U);
}

TEST_CASE("CPU executes NOP") {
    auto rig = make_cpu_with_program_16k({0xEAU}, 0x8000U);
    auto& cpu = rig.cpu;

    const usize cycles = cpu.step();

    REQUIRE(cycles == 2U);
    REQUIRE(cpu.pc() == 0x8001U);
}

TEST_CASE("CPU executes LDA immediate") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x34U});
    auto& cpu = cpu_rig.cpu;

    const usize cycles = cpu.step();

    REQUIRE(cycles == 2U);
    REQUIRE(cpu.a() == 0x34U);
    REQUIRE(cpu.pc() == 0x8002U);
}

TEST_CASE("CPU executes LDA zero page") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x44U,
        0x85U, 0x10U,
        0xA9U, 0x00U,
        0xA5U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.a() == 0x44U);
}

TEST_CASE("CPU executes LDA zero page X") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x02U,
        0xA9U, 0x66U,
        0x85U, 0x12U,
        0xA9U, 0x00U,
        0xB5U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x66U);
}

TEST_CASE("CPU executes LDA absolute") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x77U,
        0x8DU, 0x34U, 0x12U,
        0xA9U, 0x00U,
        0xADU, 0x34U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x77U);
}

TEST_CASE("CPU executes LDA absolute X and Y") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA0U, 0x02U,
        0xA9U, 0x55U,
        0x8DU, 0x21U, 0x12U,
        0xA9U, 0x66U,
        0x8DU, 0x32U, 0x12U,
        0xA9U, 0x00U,
        0xBDU, 0x20U, 0x12U,
        0xB9U, 0x30U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x55U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x66U);
}

TEST_CASE("CPU executes LDA indirect X") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x04U,
        0xA9U, 0x34U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x99U,
        0x8DU, 0x34U, 0x12U,
        0xA9U, 0x00U,
        0xA1U, 0x20U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x99U);
}

TEST_CASE("CPU executes LDA indirect Y") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA0U, 0x03U,
        0xA9U, 0x40U,
        0x85U, 0x10U,
        0xA9U, 0x12U,
        0x85U, 0x11U,
        0xA9U, 0x77U,
        0x8DU, 0x43U, 0x12U,
        0xA9U, 0x00U,
        0xB1U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0x77U);
}

TEST_CASE("CPU executes LDX and LDY from memory") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x12U,
        0x85U, 0x20U,
        0xA9U, 0x34U,
        0x8DU, 0x00U, 0x13U,
        0xA6U, 0x20U,
        0xACU, 0x00U, 0x13U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.x() == 0x12U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.y() == 0x34U);
}

TEST_CASE("CPU executes STX and STY") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0xABU,
        0xA0U, 0xCDU,
        0x86U, 0x10U,
        0x8CU, 0x34U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.bus().read(0x0010U) == 0xABU);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.bus().read(0x1234U) == 0xCDU);
}

TEST_CASE("CPU executes STA indirect X and indirect Y") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x04U,
        0xA0U, 0x03U,
        0xA9U, 0x50U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x60U,
        0x85U, 0x10U,
        0xA9U, 0x12U,
        0x85U, 0x11U,
        0xA9U, 0x5AU,
        0x81U, 0x20U,
        0x91U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1250U) == 0x5AU);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1263U) == 0x5AU);
}

TEST_CASE("CPU executes STA zero page X and absolute indexed") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x03U,
        0xA0U, 0x04U,
        0xA9U, 0x5AU,
        0x95U, 0x10U,
        0x9DU, 0x20U, 0x12U,
        0x99U, 0x30U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.bus().read(0x0013U) == 0x5AU);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x1223U) == 0x5AU);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x5AU);
}

TEST_CASE("CPU executes AND ORA EOR") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0xF0U,
        0x29U, 0x0FU,
        0x09U, 0x03U,
        0x49U, 0x01U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x00U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x03U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x02U);
}

TEST_CASE("CPU executes memory forms of AND ORA EOR") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x0FU,
        0x85U, 0x10U,
        0xA9U, 0xF0U,
        0x85U, 0x11U,
        0xA9U, 0xAAU,
        0x2DU, 0x10U, 0x00U,
        0x05U, 0x11U,
        0x45U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x0AU);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.a() == 0xFAU);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.a() == 0xF5U);
}

TEST_CASE("CPU executes CMP CPX CPY") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x20U,
        0xA2U, 0x10U,
        0xA0U, 0x08U,
        0xC9U, 0x20U,
        0xE0U, 0x0FU,
        0xC0U, 0x09U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);
}

TEST_CASE("CPU executes memory forms of CMP") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x33U,
        0x85U, 0x10U,
        0xA9U, 0x33U,
        0xC5U, 0x10U,
        0xA9U, 0x40U,
        0x8DU, 0x34U, 0x12U,
        0xA9U, 0x20U,
        0xCDU, 0x34U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);
}

TEST_CASE("CPU executes BIT zero page and absolute") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x40U,
        0x85U, 0x10U,
        0xA9U, 0x40U,
        0x24U, 0x10U,
        0xA9U, 0xC0U,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x40U,
        0x2CU, 0x00U, 0x13U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE((cpu.status() & 0x40U) == 0x40U);
    REQUIRE((cpu.status() & 0x80U) == 0x00U);
    REQUIRE((cpu.status() & 0x02U) == 0x00U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x40U) == 0x40U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);
}

TEST_CASE("CPU executes SEC and CLC") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x38U,
        0x18U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x01U) == 0x00U);
}

TEST_CASE("CPU executes ADC immediate") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x10U,
        0x18U,
        0x69U, 0x05U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x15U);
    REQUIRE((cpu.status() & 0x01U) == 0x00U);
}

TEST_CASE("CPU ADC sets carry and zero appropriately") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0xFFU,
        0x18U,
        0x69U, 0x01U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x00U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
}

TEST_CASE("CPU ADC sets overflow on signed overflow") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x50U,
        0x18U,
        0x69U, 0x50U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0xA0U);
    REQUIRE((cpu.status() & 0x40U) == 0x40U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);
}

TEST_CASE("CPU executes SBC immediate") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x10U,
        0x38U,
        0xE9U, 0x03U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x0DU);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);
}

TEST_CASE("CPU SBC clears carry on borrow") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x10U,
        0x38U,
        0xE9U, 0x20U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0xF0U);
    REQUIRE((cpu.status() & 0x01U) == 0x00U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);
}

TEST_CASE("CPU executes TAX and TXA") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x12U, 0xAAU, 0x8AU});
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.x() == 0x12U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x12U);
}

TEST_CASE("CPU executes TAY and TYA") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x34U, 0xA8U, 0x98U});
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.y() == 0x34U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x34U);
}

TEST_CASE("CPU executes TSX and TXS") {
    auto cpu_rig = make_cpu_with_program_16k({0xA2U, 0x77U, 0x9AU, 0xBAU});
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.sp() == 0x77U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.x() == 0x77U);
}

TEST_CASE("CPU executes INX DEX INY DEY") {
    auto cpu_rig = make_cpu_with_program_16k({0xE8U, 0xCAU, 0xC8U, 0x88U});
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.x() == 0x01U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.x() == 0x00U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.y() == 0x01U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.y() == 0x00U);
}

TEST_CASE("CPU executes memory INC and DEC") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x01U,
        0x85U, 0x10U,
        0xE6U, 0x10U,
        0xC6U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x0010U) == 0x02U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x0010U) == 0x01U);
}

TEST_CASE("CPU executes ASL and LSR accumulator") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x81U,
        0x0AU,
        0x4AU
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x02U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x01U);
}

TEST_CASE("CPU executes ROL and ROR accumulator") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x38U,
        0xA9U, 0x40U,
        0x2AU,
        0x6AU
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x81U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x40U);
}

TEST_CASE("CPU executes memory shifts and rotates") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x81U,
        0x85U, 0x10U,
        0x06U, 0x10U,
        0x46U, 0x10U,
        0x38U,
        0x26U, 0x10U,
        0x66U, 0x10U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x0010U) == 0x02U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x0010U) == 0x01U);

    REQUIRE(cpu.step() == 2U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x0010U) == 0x03U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x0010U) == 0x01U);
}

TEST_CASE("CPU executes PHA and PLA") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x5AU, 0x48U, 0xA9U, 0x00U, 0x68U});
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x5AU);

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.sp() == 0xFCU);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x00U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x5AU);
    REQUIRE(cpu.sp() == 0xFDU);
}

TEST_CASE("CPU executes PHP and PLP") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x00U, 0x08U, 0xA9U, 0x01U, 0x28U});
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.sp() == 0xFCU);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x02U) == 0x00U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
    REQUIRE(cpu.sp() == 0xFDU);
}

TEST_CASE("CPU executes BEQ when zero flag is set") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x00U,
        0xF0U, 0x02U,
        0xA9U, 0x11U,
        0xA9U, 0x22U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x22U);
}

TEST_CASE("CPU does not take BNE when zero flag is set") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x00U,
        0xD0U, 0x02U,
        0xA9U, 0x33U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x33U);
}

TEST_CASE("CPU executes BMI when negative flag is set") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x80U,
        0x30U, 0x02U,
        0xA9U, 0x11U,
        0xA9U, 0x44U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x44U);
}

TEST_CASE("CPU executes BPL when negative flag is clear") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x01U,
        0x10U, 0x02U,
        0xA9U, 0x11U,
        0xA9U, 0x55U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x55U);
}

TEST_CASE("CPU executes JSR and RTS") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x20U, 0x06U, 0x80U,
        0xA9U, 0x11U,
        0xEAU,
        0xA9U, 0x77U,
        0x60U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.pc() == 0x8006U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x77U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.pc() == 0x8003U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x11U);
}

TEST_CASE("CPU executes JMP indirect with 6502 page-wrap bug") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x34U,
        0x8DU, 0xFFU, 0x12U,
        0xA9U, 0x80U,
        0x8DU, 0x00U, 0x12U,
        0x6CU, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.pc() == 0x8034U);
}

TEST_CASE("CPU executes BCC and BCS based on carry flag") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x90U, 0x02U,
        0xA9U, 0x11U,
        0xA9U, 0x22U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x22U);
}

TEST_CASE("CPU logging writes a trace line") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x42U});
    auto& cpu = cpu_rig.cpu;
    std::ostringstream log;

    cpu.set_logging(true, &log);
    REQUIRE(cpu.step() == 2U);

    const std::string text = log.str();
    REQUIRE_FALSE(text.empty());
    REQUIRE(text.find("PC=8000") != std::string::npos);
    REQUIRE(text.find("OP=A9") != std::string::npos);
}

TEST_CASE("CPU logging can be disabled") {
    auto cpu_rig = make_cpu_with_program_16k({0xEAU});
    auto& cpu = cpu_rig.cpu;
    std::ostringstream log;

    cpu.set_logging(false, &log);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(log.str().empty());
}

TEST_CASE("CPU trace_line reports current state before execution") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x42U});
    auto& cpu = cpu_rig.cpu;

    const std::string line = cpu.trace_line();

    REQUIRE(line.find("PC=8000") != std::string::npos);
    REQUIRE(line.find("OP=A9") != std::string::npos);
    REQUIRE(line.find("A=00") != std::string::npos);
    REQUIRE(line.find("X=00") != std::string::npos);
    REQUIRE(line.find("Y=00") != std::string::npos);
    REQUIRE(line.find("SP=FD") != std::string::npos);
}

TEST_CASE("CPU logging writes one line per executed instruction") {
    auto cpu_rig = make_cpu_with_program_16k({0xA9U, 0x42U, 0xEAU});
    auto& cpu = cpu_rig.cpu;
    std::ostringstream log;

    cpu.set_logging(true, &log);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);

    const std::string text = log.str();
    REQUIRE_FALSE(text.empty());
    REQUIRE(text.find("PC=8000") != std::string::npos);
    REQUIRE(text.find("OP=A9") != std::string::npos);
    REQUIRE(text.find("PC=8002") != std::string::npos);
    REQUIRE(text.find("OP=EA") != std::string::npos);
}

TEST_CASE("CPU set_pc overrides program counter") {
    auto cpu_rig = make_cpu_with_program_16k({0xEAU}, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    cpu.set_pc(0xC000U);

    REQUIRE(cpu.pc() == 0xC000U);
}

TEST_CASE("CPU trace_line reflects forced program counter") {
    auto cpu_rig = make_cpu_with_program_16k({0xEAU}, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    cpu.set_pc(0x8000U);

    const std::string line = cpu.trace_line();

    REQUIRE(line.find("PC=8000") != std::string::npos);
}

TEST_CASE("CPU logging still works after forcing PC") {
    auto cpu_rig = make_cpu_with_program_32k({0xEAU}, 0xC000U);
    auto& cpu = cpu_rig.cpu;
    std::ostringstream log;

    cpu.set_pc(0xC000U);
    cpu.set_logging(true, &log);

    REQUIRE(cpu.step() == 2U);

    const std::string text = log.str();
    REQUIRE(text.find("PC=C000") != std::string::npos);
    REQUIRE(text.find("OP=EA") != std::string::npos);
}

TEST_CASE("CPU executes ORA zero page X absolute X absolute Y indirect X indirect Y") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA0U, 0x02U,

        0xA9U, 0x0FU,
        0x85U, 0x31U,

        0xA9U, 0xF0U,
        0x8DU, 0x21U, 0x12U,

        0xA9U, 0x33U,
        0x8DU, 0x32U, 0x12U,

        0xA9U, 0x40U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x44U,
        0x8DU, 0x40U, 0x12U,

        0xA9U, 0x50U,
        0x85U, 0x40U,
        0xA9U, 0x12U,
        0x85U, 0x41U,
        0xA9U, 0x88U,
        0x8DU, 0x52U, 0x12U,

        0xA9U, 0x00U,
        0x15U, 0x30U,
        0x1DU, 0x20U, 0x12U,
        0x19U, 0x30U, 0x12U,
        0x01U, 0x23U,
        0x11U, 0x40U
    });
    auto& cpu = cpu_rig.cpu;

    for (int i = 0; i < 21; ++i) {
        REQUIRE(cpu.step() >= 2U);
    }

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x0FU);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0xFFU);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0xFFU);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0xFFU);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0xFFU);
}

TEST_CASE("CPU executes AND zero page X absolute X absolute Y indirect X indirect Y") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA0U, 0x02U,

        0xA9U, 0xF0U,
        0x85U, 0x31U,

        0xA9U, 0xCCU,
        0x8DU, 0x21U, 0x12U,

        0xA9U, 0x0FU,
        0x8DU, 0x32U, 0x12U,

        0xA9U, 0x40U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0xAAU,
        0x8DU, 0x40U, 0x12U,

        0xA9U, 0x50U,
        0x85U, 0x40U,
        0xA9U, 0x12U,
        0x85U, 0x41U,
        0xA9U, 0x55U,
        0x8DU, 0x52U, 0x12U,

        0xA9U, 0xFFU,
        0x35U, 0x30U,
        0x3DU, 0x20U, 0x12U,
        0x39U, 0x30U, 0x12U,
        0x21U, 0x23U,
        0x31U, 0x40U
    });
    auto& cpu = cpu_rig.cpu;

    for (int i = 0; i < 21; ++i) {
        REQUIRE(cpu.step() >= 2U);
    }

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0xF0U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0xC0U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x00U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x00U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0x00U);
}

TEST_CASE("CPU executes EOR zero page X absolute X absolute Y indirect X indirect Y") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA0U, 0x02U,

        0xA9U, 0x0FU,
        0x85U, 0x31U,

        0xA9U, 0xF0U,
        0x8DU, 0x21U, 0x12U,

        0xA9U, 0xAAU,
        0x8DU, 0x32U, 0x12U,

        0xA9U, 0x40U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x33U,
        0x8DU, 0x40U, 0x12U,

        0xA9U, 0x50U,
        0x85U, 0x40U,
        0xA9U, 0x12U,
        0x85U, 0x41U,
        0xA9U, 0x77U,
        0x8DU, 0x52U, 0x12U,

        0xA9U, 0xFFU,
        0x55U, 0x30U,
        0x5DU, 0x20U, 0x12U,
        0x59U, 0x30U, 0x12U,
        0x41U, 0x23U,
        0x51U, 0x40U
    });
    auto& cpu = cpu_rig.cpu;

    for (int i = 0; i < 21; ++i) {
        REQUIRE(cpu.step() >= 2U);
    }

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0xF0U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x00U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0xAAU);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x99U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0xEEU);
}

TEST_CASE("CPU executes ADC memory addressing modes") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x18U,
        0xA9U, 0x10U,

        0xA9U, 0x05U,
        0x85U, 0x30U,

        0xA2U, 0x01U,
        0xA0U, 0x02U,

        0xA9U, 0x06U,
        0x8DU, 0x20U, 0x12U,

        0xA9U, 0x07U,
        0x8DU, 0x31U, 0x12U,

        0xA9U, 0x08U,
        0x8DU, 0x42U, 0x12U,

        0xA9U, 0x50U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x09U,
        0x8DU, 0x50U, 0x12U,

        0xA9U, 0x60U,
        0x85U, 0x40U,
        0xA9U, 0x12U,
        0x85U, 0x41U,
        0xA9U, 0x0AU,
        0x8DU, 0x62U, 0x12U,

        0xA9U, 0x10U,
        0x65U, 0x30U,
        0x75U, 0x2FU,
        0x6DU, 0x20U, 0x12U,
        0x7DU, 0x30U, 0x12U,
        0x79U, 0x40U, 0x12U,
        0x61U, 0x23U,
        0x71U, 0x40U
    });
    auto& cpu = cpu_rig.cpu;

    for (int i = 0; i < 24; ++i) {
        REQUIRE(cpu.step() >= 2U);
    }

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x10U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.a() == 0x15U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x1AU);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x20U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x27U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x2FU);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x38U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0x42U);
}

TEST_CASE("CPU executes SBC memory addressing modes") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x38U,
        0xA9U, 0x40U,

        0xA9U, 0x05U,
        0x85U, 0x30U,

        0xA2U, 0x01U,
        0xA0U, 0x02U,

        0xA9U, 0x06U,
        0x8DU, 0x20U, 0x12U,

        0xA9U, 0x07U,
        0x8DU, 0x31U, 0x12U,

        0xA9U, 0x08U,
        0x8DU, 0x42U, 0x12U,

        0xA9U, 0x50U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x09U,
        0x8DU, 0x50U, 0x12U,

        0xA9U, 0x60U,
        0x85U, 0x40U,
        0xA9U, 0x12U,
        0x85U, 0x41U,
        0xA9U, 0x0AU,
        0x8DU, 0x62U, 0x12U,

        0xA9U, 0x40U,
        0xE5U, 0x30U,
        0xF5U, 0x2FU,
        0xEDU, 0x20U, 0x12U,
        0xFDU, 0x30U, 0x12U,
        0xF9U, 0x40U, 0x12U,
        0xE1U, 0x23U,
        0xF1U, 0x40U
    });
    auto& cpu = cpu_rig.cpu;

    for (int i = 0; i < 24; ++i) {
        REQUIRE(cpu.step() >= 2U);
    }

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x40U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.a() == 0x3BU);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x36U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x30U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x29U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.a() == 0x21U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x18U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0x0EU);
}

TEST_CASE("CPU executes CMP indexed and indirect addressing modes") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x40U,
        0xA2U, 0x01U,
        0xA0U, 0x02U,

        0xA9U, 0x40U,
        0x85U, 0x31U,

        0xA9U, 0x40U,
        0x8DU, 0x21U, 0x12U,

        0xA9U, 0x40U,
        0x8DU, 0x32U, 0x12U,

        0xA9U, 0x50U,
        0x85U, 0x24U,
        0xA9U, 0x12U,
        0x85U, 0x25U,
        0xA9U, 0x41U,
        0x8DU, 0x50U, 0x12U,

        0xA9U, 0x60U,
        0x85U, 0x40U,
        0xA9U, 0x12U,
        0x85U, 0x41U,
        0xA9U, 0x50U,
        0x8DU, 0x62U, 0x12U,

        0xA9U, 0x40U,
        0xD5U, 0x30U,
        0xDDU, 0x20U, 0x12U,
        0xD9U, 0x30U, 0x12U,
        0xC1U, 0x23U,
        0xD1U, 0x40U
    });
    auto& cpu = cpu_rig.cpu;

    for (int i = 0; i < 21; ++i) {
        REQUIRE(cpu.step() >= 2U);
    }

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x40U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE((cpu.status() & 0x01U) == 0x00U);

    REQUIRE(cpu.step() == 5U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);
}

TEST_CASE("CPU executes CPX and CPY zero page and absolute") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x10U,
        0xA0U, 0x20U,
        0xA9U, 0x10U,
        0x85U, 0x10U,
        0xA9U, 0x20U,
        0x8DU, 0x34U, 0x12U,
        0xE4U, 0x10U,
        0xECU, 0x34U, 0x12U,
        0xC4U, 0x10U,
        0xCCU, 0x34U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x80U) == 0x80U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE((cpu.status() & 0x01U) == 0x01U);

    REQUIRE(cpu.step() == 4U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
}

TEST_CASE("CPU executes CLI SEI CLD SED CLV") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x78U,
        0x58U,
        0xF8U,
        0xD8U,
        0xA9U, 0x50U,
        0x18U,
        0x69U, 0x50U,
        0xB8U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x04U) == 0x04U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x04U) == 0x00U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x08U) == 0x08U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x08U) == 0x00U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x40U) == 0x40U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x40U) == 0x00U);
}

TEST_CASE("CPU executes BVC and BVS") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x50U,
        0x18U,
        0x69U, 0x50U,
        0x70U, 0x02U,
        0xA9U, 0x11U,
        0xA9U, 0x22U,
        0xB8U,
        0x50U, 0x02U,
        0xA9U, 0x33U,
        0xA9U, 0x44U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x40U) == 0x40U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x22U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x40U) == 0x00U);

    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.a() == 0x44U);
}
TEST_CASE("CPU executes BRK and jumps to IRQ BRK vector") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0x00U,
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x1234U
    );
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.pc() == 0x1234U);
    REQUIRE((cpu.status() & 0x04U) == 0x04U);
}

TEST_CASE("CPU executes RTI and restores status and PC") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0x00U,
            0xEAU,
            0x40U
        },
        0xC000U,
        0xC000U,
        0xC002U
    );
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.pc() == 0xC002U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.pc() == 0xC002U);
    REQUIRE((cpu.status() & 0x04U) == 0x04U);
}

TEST_CASE("CPU BRK pushes return address and status to stack") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0x00U,
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x8000U
    );
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 7U);

    REQUIRE(cpu.bus().read(0x01FDU) == 0xC0U);
    REQUIRE(cpu.bus().read(0x01FCU) == 0x02U);
    REQUIRE((cpu.bus().read(0x01FBU) & 0x10U) == 0x10U);
}

TEST_CASE("CPU IRQ is ignored when interrupt disable flag is set") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x3456U
    );
    auto& cpu = cpu_rig.cpu;

    cpu.irq();
    REQUIRE(cpu.pc() == 0xC000U);
}

TEST_CASE("CPU IRQ enters vector when interrupt disable flag is clear") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0x58U,
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x3456U
    );
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE((cpu.status() & 0x04U) == 0x00U);

    cpu.irq();
    REQUIRE(cpu.pc() == 0x3456U);
    REQUIRE((cpu.status() & 0x04U) == 0x04U);
}

TEST_CASE("CPU NMI always enters NMI vector") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x3456U,
        0x4567U
    );
    auto& cpu = cpu_rig.cpu;

    cpu.nmi();
    REQUIRE(cpu.pc() == 0x4567U);
    REQUIRE((cpu.status() & 0x04U) == 0x04U);
}

TEST_CASE("CPU nestest like trace line contains mnemonic and registers") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x42U
    }, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    const std::string line = cpu.trace_line(Cpu::TraceFormat::NestestLike);

    REQUIRE(line.find("8000") != std::string::npos);
    REQUIRE(line.find("A9 42") != std::string::npos);
    REQUIRE(line.find("LDA #$42") != std::string::npos);
    REQUIRE(line.find("A:00") != std::string::npos);
    REQUIRE(line.find("SP:FD") != std::string::npos);
}

TEST_CASE("CPU nestest like logging writes formatted line") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xEAU
    }, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    std::ostringstream log;
    cpu.set_logging(true, &log, Cpu::TraceFormat::NestestLike);

    REQUIRE(cpu.step() == 2U);

    const std::string text = log.str();
    REQUIRE(text.find("8000") != std::string::npos);
    REQUIRE(text.find("NOP") != std::string::npos);
    REQUIRE(text.find("A:00") != std::string::npos);
}
TEST_CASE("CPU absolute X addressing reads across page boundary correctly") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA9U, 0xABU,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x00U,
        0xBDU, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0xABU);
}

TEST_CASE("CPU absolute Y addressing reads across page boundary correctly") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA0U, 0x01U,
        0xA9U, 0xCDU,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x00U,
        0xB9U, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0xCDU);
}

TEST_CASE("CPU indirect Y addressing reads across page boundary correctly") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA0U, 0x01U,
        0xA9U, 0xFFU,
        0x85U, 0x20U,
        0xA9U, 0x12U,
        0x85U, 0x21U,
        0xA9U, 0x77U,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x00U,
        0xB1U, 0x20U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x77U);
}

TEST_CASE("CPU absolute X store writes across page boundary correctly") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA9U, 0x5AU,
        0x9DU, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.bus().read(0x1300U) == 0x5AU);
}

TEST_CASE("CPU BRK stack frame includes break flag") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0x00U,
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x8000U
    );
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 7U);
    REQUIRE((cpu.bus().read(0x01FBU) & 0x10U) == 0x10U);
}

TEST_CASE("CPU IRQ stack frame clears break flag") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0x58U,
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x3456U
    );
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    cpu.irq();

    REQUIRE(cpu.pc() == 0x3456U);
    REQUIRE((cpu.bus().read(0x01FBU) & 0x10U) == 0x00U);
}

TEST_CASE("CPU NMI stack frame clears break flag") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x3456U,
        0x4567U
    );
    auto& cpu = cpu_rig.cpu;

    cpu.nmi();

    REQUIRE(cpu.pc() == 0x4567U);
    REQUIRE((cpu.bus().read(0x01FBU) & 0x10U) == 0x00U);
}

TEST_CASE("CPU nestest like trace formats branch target") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xF0U, 0x02U
    }, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    const std::string line = cpu.trace_line(Cpu::TraceFormat::NestestLike);

    REQUIRE(line.find("F0 02") != std::string::npos);
    REQUIRE(line.find("BEQ $8004") != std::string::npos);
}

TEST_CASE("CPU nestest like trace formats zero page and absolute operands") {
    auto cpu_zp_rig = make_cpu_with_program_16k({
        0xA5U, 0x10U
    }, 0x8000U);
    auto& cpu_zp = cpu_zp_rig.cpu;
    auto cpu_abs_rig = make_cpu_with_program_16k({
        0xADU, 0x34U, 0x12U
    }, 0x8000U);
    auto& cpu_abs = cpu_abs_rig.cpu;

    const std::string zp_line = cpu_zp.trace_line(Cpu::TraceFormat::NestestLike);
    const std::string abs_line = cpu_abs.trace_line(Cpu::TraceFormat::NestestLike);

    REQUIRE(zp_line.find("A5 10") != std::string::npos);
    REQUIRE(zp_line.find("LDA $10") != std::string::npos);

    REQUIRE(abs_line.find("AD 34 12") != std::string::npos);
    REQUIRE(abs_line.find("LDA $1234") != std::string::npos);
}

TEST_CASE("CPU LDA absolute X adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA9U, 0xABU,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x00U,
        0xBDU, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0xABU);
}

TEST_CASE("CPU branch taken across page adds extra cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x00U,
        0xF0U, 0x7FU
    }, 0x807EU);
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.pc() == 0x8101U);
}

TEST_CASE("CPU branch taken same page costs three cycles") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x00U,
        0xF0U, 0x02U
    }, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.pc() == 0x8006U);
}

TEST_CASE("CPU services PPU NMI after instruction completes") {
    auto cpu_rig = make_cpu_with_patched_program_32k(
        {
            0xEAU
        },
        0xC000U,
        0xC000U,
        0x9000U,
        0xC100U,
        {
            {0xC100U, 0xEAU}
        }
    );
    auto& cpu = cpu_rig.cpu;

    cpu.bus().write(0x2000U, 0x80U);
    cpu.bus().ppu().tick(242U * 341U + 2U);

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.pc() == 0xC100U);
}

TEST_CASE("CPU nestest like trace formats BEQ target") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xF0U, 0x02U
    }, 0x8000U);
    auto& cpu = cpu_rig.cpu;

    const std::string line = cpu.trace_line(Cpu::TraceFormat::NestestLike);

    REQUIRE(line.find("F0 02") != std::string::npos);
    REQUIRE(line.find("BEQ $8004") != std::string::npos);
}
TEST_CASE("CPU executes absolute memory shifts and rotates") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x81U,
        0x8DU, 0x34U, 0x12U,
        0x0EU, 0x34U, 0x12U,
        0x4EU, 0x34U, 0x12U,
        0x38U,
        0x2EU, 0x34U, 0x12U,
        0x6EU, 0x34U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x02U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x01U);

    REQUIRE(cpu.step() == 2U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x03U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x01U);
}

TEST_CASE("CPU executes absolute X memory shifts and rotates") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA9U, 0x81U,
        0x8DU, 0x35U, 0x12U,
        0x1EU, 0x34U, 0x12U,
        0x5EU, 0x34U, 0x12U,
        0x38U,
        0x3EU, 0x34U, 0x12U,
        0x7EU, 0x34U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.bus().read(0x1235U) == 0x02U);

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.bus().read(0x1235U) == 0x01U);

    REQUIRE(cpu.step() == 2U);

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.bus().read(0x1235U) == 0x03U);

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.bus().read(0x1235U) == 0x01U);
}

TEST_CASE("CPU executes absolute and absolute X INC DEC") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA9U, 0x01U,
        0x8DU, 0x34U, 0x12U,
        0xA2U, 0x01U,
        0xA9U, 0x05U,
        0x8DU, 0x41U, 0x12U,
        0xEEU, 0x34U, 0x12U,
        0xCEU, 0x34U, 0x12U,
        0xFEU, 0x40U, 0x12U,
        0xDEU, 0x40U, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x02U);

    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.bus().read(0x1234U) == 0x01U);

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.bus().read(0x1241U) == 0x06U);

    REQUIRE(cpu.step() == 7U);
    REQUIRE(cpu.bus().read(0x1241U) == 0x05U);
}
TEST_CASE("CPU ORA absolute X adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA2U, 0x01U,
        0xA9U, 0x0FU,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0xF0U,
        0x1DU, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0xFFU);
}

TEST_CASE("CPU AND absolute Y adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA0U, 0x01U,
        0xA9U, 0x0FU,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0xF3U,
        0x39U, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0x03U);
}

TEST_CASE("CPU EOR indirect Y adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA0U, 0x01U,
        0xA9U, 0xFFU,
        0x85U, 0x20U,
        0xA9U, 0x12U,
        0x85U, 0x21U,
        0xA9U, 0x0FU,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0xF0U,
        0x51U, 0x20U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0xFFU);
}

TEST_CASE("CPU ADC indirect Y adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x18U,
        0xA0U, 0x01U,
        0xA9U, 0xFFU,
        0x85U, 0x20U,
        0xA9U, 0x12U,
        0x85U, 0x21U,
        0xA9U, 0x10U,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x20U,
        0x71U, 0x20U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 3U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 6U);
    REQUIRE(cpu.a() == 0x30U);
}

TEST_CASE("CPU SBC absolute X adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0x38U,
        0xA2U, 0x01U,
        0xA9U, 0x05U,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x20U,
        0xFDU, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE(cpu.a() == 0x1BU);
}

TEST_CASE("CPU CMP absolute Y adds page cross cycle") {
    auto cpu_rig = make_cpu_with_program_16k({
        0xA0U, 0x01U,
        0xA9U, 0x40U,
        0xA9U, 0x40U,
        0x8DU, 0x00U, 0x13U,
        0xA9U, 0x40U,
        0xD9U, 0xFFU, 0x12U
    });
    auto& cpu = cpu_rig.cpu;

    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 4U);
    REQUIRE(cpu.step() == 2U);
    REQUIRE(cpu.step() == 5U);
    REQUIRE((cpu.status() & 0x02U) == 0x02U);
}