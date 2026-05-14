#include <catch2/catch_test_macros.hpp>

#include "mappers/mapper9.hpp"
#include "rom.hpp"

using namespace nes;

namespace {

Rom make_mapper9_rom() {
    Rom rom {};
    rom.mapper_id = 9;
    rom.mirroring = Mirroring::Vertical;

    rom.prg_rom.assign(8U * 1024U * 16U, 0);
    rom.chr_data.assign(4U * 1024U * 32U, 0);
    rom.chr_is_ram = false;

    for (usize bank = 0; bank < 16U; ++bank) {
        rom.prg_rom[bank * 8U * 1024U] = static_cast<u8>(bank);
    }

    for (usize bank = 0; bank < 32U; ++bank) {
        rom.chr_data[bank * 4U * 1024U] = static_cast<u8>(bank);
    }

    return rom;
}

} // namespace

TEST_CASE("Mapper9 PRG bank at 0x8000 is switchable and upper banks are fixed") {
    Mapper9 mapper(make_mapper9_rom());

    mapper.cpu_write(0xA000U, 0x04U);

    REQUIRE(mapper.cpu_read(0x8000U) == 0x04U);
    REQUIRE(mapper.cpu_read(0xA000U) == 0x0DU);
    REQUIRE(mapper.cpu_read(0xC000U) == 0x0EU);
    REQUIRE(mapper.cpu_read(0xE000U) == 0x0FU);
}

TEST_CASE("Mapper9 mirroring is switchable") {
    Mapper9 mapper(make_mapper9_rom());

    mapper.cpu_write(0xF000U, 0x00U);
    REQUIRE(mapper.mirroring() == Mirroring::Vertical);

    mapper.cpu_write(0xF000U, 0x01U);
    REQUIRE(mapper.mirroring() == Mirroring::Horizontal);
}

TEST_CASE("Mapper9 lower CHR latch switches on 0x0FD8 and 0x0FE8") {
    auto rom = make_mapper9_rom();
    Mapper9 mapper(std::move(rom));

    mapper.cpu_write(0xB000U, 0x03U); // lower FD bank
    mapper.cpu_write(0xC000U, 0x07U); // lower FE bank

    mapper.ppu_read(0x0FD8U);
    REQUIRE(mapper.ppu_read(0x0000U) == 0x03U);

    mapper.ppu_read(0x0FE8U);
    REQUIRE(mapper.ppu_read(0x0000U) == 0x07U);
}

TEST_CASE("Mapper9 upper CHR latch switches on 0x1FD8-0x1FDF and 0x1FE8-0x1FEF") {
    auto rom = make_mapper9_rom();
    Mapper9 mapper(std::move(rom));

    mapper.cpu_write(0xD000U, 0x0AU); // upper FD bank
    mapper.cpu_write(0xE000U, 0x0BU); // upper FE bank

    mapper.ppu_read(0x1FD8U);
    REQUIRE(mapper.ppu_read(0x1000U) == 0x0AU);

    mapper.ppu_read(0x1FEFU);
    REQUIRE(mapper.ppu_read(0x1000U) == 0x0BU);
}