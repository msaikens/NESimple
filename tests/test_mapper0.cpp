#include <catch2/catch_test_macros.hpp>

#include "./mappers/mapper0.hpp"

using namespace nes;

static Rom make_nrom_16k_rom() {
    Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(16 * 1024, 0);
    rom.chr_data.assign(8 * 1024, 0);
    rom.chr_is_ram = true;

    // Marker bytes at the start and end of the 16 KB PRG bank.
    rom.prg_rom[0x0000] = 0x11;
    rom.prg_rom[0x3FFF] = 0x22;

    return rom;
}

TEST_CASE("Mapper0 mirrors 16KB PRG ROM into both PRG regions") {
    Mapper0 mapper(make_nrom_16k_rom());

    REQUIRE(mapper.cpu_read(0x8000) == 0x11);
    REQUIRE(mapper.cpu_read(0xBFFF) == 0x22);
    REQUIRE(mapper.cpu_read(0xC000) == 0x11);
    REQUIRE(mapper.cpu_read(0xFFFF) == 0x22);
}

TEST_CASE("Mapper0 allows writes when CHR is RAM") {
    Mapper0 mapper(make_nrom_16k_rom());

    mapper.ppu_write(0x0010, 0x77);
    REQUIRE(mapper.ppu_read(0x0010) == 0x77);
}