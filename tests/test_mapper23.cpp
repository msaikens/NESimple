#include <catch2/catch_test_macros.hpp>

#include "./mappers/mapper23.hpp"

using namespace nes;

namespace {

Rom make_mapper23_rom() {
    Rom rom {};
    rom.mapper_id = 23;
    rom.mirroring = Mirroring::Vertical;
    rom.chr_is_ram = false;

    rom.prg_rom.resize(8U * 8U * 1024U);
    rom.chr_data.resize(16U * 1024U);

    for (usize bank = 0; bank < 8U; ++bank) {
        const usize base = bank * 8U * 1024U;
        rom.prg_rom[base] = static_cast<u8>(0x10U + bank);
        rom.prg_rom[base + 0x1FFFU] = static_cast<u8>(0x20U + bank);
    }

    for (usize bank = 0; bank < 16U; ++bank) {
        const usize base = bank * 1024U;
        rom.chr_data[base] = static_cast<u8>(0x40U + bank);
        rom.chr_data[base + 0x03FFU] = static_cast<u8>(0x80U + bank);
    }

    return rom;
}

} // namespace

TEST_CASE("Mapper23 starts with switchable PRG banks and fixed final banks") {
    Mapper23 mapper(make_mapper23_rom());

    REQUIRE(mapper.cpu_read(0x8000U) == 0x10U);
    REQUIRE(mapper.cpu_read(0xA000U) == 0x11U);
    REQUIRE(mapper.cpu_read(0xC000U) == 0x16U);
    REQUIRE(mapper.cpu_read(0xE000U) == 0x17U);
}

TEST_CASE("Mapper23 switches 8000 and A000 PRG banks") {
    Mapper23 mapper(make_mapper23_rom());

    mapper.cpu_write(0x8000U, 0x03U);
    mapper.cpu_write(0xA000U, 0x04U);

    REQUIRE(mapper.cpu_read(0x8000U) == 0x13U);
    REQUIRE(mapper.cpu_read(0xA000U) == 0x14U);
    REQUIRE(mapper.cpu_read(0xC000U) == 0x16U);
    REQUIRE(mapper.cpu_read(0xE000U) == 0x17U);
}

TEST_CASE("Mapper23 maps eight 1KB CHR banks") {
    Mapper23 mapper(make_mapper23_rom());

    mapper.cpu_write(0xB000U, 0x05U);
    mapper.cpu_write(0xB001U, 0x00U);

    mapper.cpu_write(0xB002U, 0x06U);
    mapper.cpu_write(0xB003U, 0x00U);

    REQUIRE(mapper.ppu_read(0x0000U) == 0x45U);
    REQUIRE(mapper.ppu_read(0x0400U) == 0x46U);
}

TEST_CASE("Mapper23 VRC2 latch covers cartridge RAM address range") {
    Mapper23 mapper(make_mapper23_rom());

    mapper.cpu_write(0x7000U, 0x01U);
    REQUIRE((mapper.cpu_read(0x6000U) & 0x01U) == 0x01U);
    REQUIRE((mapper.cpu_read(0x7000U) & 0x01U) == 0x01U);

    mapper.cpu_write(0x7000U, 0x00U);
    REQUIRE((mapper.cpu_read(0x6000U) & 0x01U) == 0x00U);
    REQUIRE((mapper.cpu_read(0x7000U) & 0x01U) == 0x00U);
}
TEST_CASE("Mapper23 maps CHR register address pairs explicitly") {
    Mapper23 mapper(make_mapper23_rom());

    mapper.cpu_write(0xB000U, 0x05U);
    mapper.cpu_write(0xB001U, 0x00U);

    mapper.cpu_write(0xB002U, 0x06U);
    mapper.cpu_write(0xB003U, 0x00U);

    mapper.cpu_write(0xC000U, 0x07U);
    mapper.cpu_write(0xC001U, 0x00U);

    REQUIRE(mapper.ppu_read(0x0000U) == 0x45U);
    REQUIRE(mapper.ppu_read(0x0400U) == 0x46U);
    REQUIRE(mapper.ppu_read(0x0800U) == 0x47U);
}
