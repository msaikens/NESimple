#include <catch2/catch_test_macros.hpp>

#include "./mappers/mapper2.hpp"

using namespace nes;

namespace {

Rom make_mapper2_rom() {
    Rom rom {};
    rom.mapper_id = 2;
    rom.mirroring = Mirroring::Horizontal;
    rom.chr_is_ram = false;

    rom.prg_rom.resize(4U * 16U * 1024U);
    rom.chr_data.resize(8U * 1024U);

    for (usize bank = 0; bank < 4U; ++bank) {
        const usize bank_start = bank * 16U * 1024U;
        rom.prg_rom[bank_start] = static_cast<u8>(0x10U + bank);
        rom.prg_rom[bank_start + 0x3FFFU] = static_cast<u8>(0x20U + bank);
    }

    rom.chr_data[0x0000U] = 0x55U;
    rom.chr_data[0x1FFFU] = 0xAAU;

    return rom;
}

} // namespace

TEST_CASE("Mapper2 starts with bank 0 at 8000 and fixed last bank at C000") {
    Mapper2 mapper(make_mapper2_rom());

    REQUIRE(mapper.cpu_read(0x8000U) == 0x10U);
    REQUIRE(mapper.cpu_read(0xBFFFU) == 0x20U);

    REQUIRE(mapper.cpu_read(0xC000U) == 0x13U);
    REQUIRE(mapper.cpu_read(0xFFFFU) == 0x23U);
}

TEST_CASE("Mapper2 CPU writes switch the lower PRG bank") {
    Mapper2 mapper(make_mapper2_rom());

    mapper.cpu_write(0x8000U, 0x02U);

    REQUIRE(mapper.cpu_read(0x8000U) == 0x12U);
    REQUIRE(mapper.cpu_read(0xBFFFU) == 0x22U);

    REQUIRE(mapper.cpu_read(0xC000U) == 0x13U);
    REQUIRE(mapper.cpu_read(0xFFFFU) == 0x23U);
}

TEST_CASE("Mapper2 bank select wraps to available PRG banks") {
    Mapper2 mapper(make_mapper2_rom());

    mapper.cpu_write(0x8000U, 0x05U);

    REQUIRE(mapper.cpu_read(0x8000U) == 0x11U);
}

TEST_CASE("Mapper2 reads CHR data") {
    Mapper2 mapper(make_mapper2_rom());

    REQUIRE(mapper.ppu_read(0x0000U) == 0x55U);
    REQUIRE(mapper.ppu_read(0x1FFFU) == 0xAAU);
}

TEST_CASE("Mapper2 allows CHR RAM writes when CHR is RAM") {
    Rom rom = make_mapper2_rom();
    rom.chr_is_ram = true;
    rom.chr_data.assign(8U * 1024U, 0x00U);

    Mapper2 mapper(std::move(rom));

    mapper.ppu_write(0x0000U, 0x77U);

    REQUIRE(mapper.ppu_read(0x0000U) == 0x77U);
}

TEST_CASE("Mapper2 ignores CHR writes when CHR is ROM") {
    Mapper2 mapper(make_mapper2_rom());

    mapper.ppu_write(0x0000U, 0x77U);

    REQUIRE(mapper.ppu_read(0x0000U) == 0x55U);
}