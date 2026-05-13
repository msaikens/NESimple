#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "rom.hpp"

using namespace nes;

TEST_CASE("ROM parser reads valid iNES file safely") {
    std::vector<u8> data(16U + 16U * 1024U + 8U * 1024U, 0);

    data[0] = 'N';
    data[1] = 'E';
    data[2] = 'S';
    data[3] = 0x1A;

    data[4] = 1;
    data[5] = 1;

    data[6] = 0x01;
    data[7] = 0x00;

    data[16] = 0xAA;
    data[16U + 16U * 1024U] = 0xBB;

    const Rom rom = Rom::parse(data);

    REQUIRE(rom.mapper_id == 0);
    REQUIRE(rom.mirroring == Mirroring::Vertical);
    REQUIRE(rom.prg_rom.size() == 16U * 1024U);
    REQUIRE(rom.chr_data.size() == 8U * 1024U);
    REQUIRE(rom.prg_rom[0] == 0xAA);
    REQUIRE(rom.chr_data[0] == 0xBB);
    REQUIRE_FALSE(rom.chr_is_ram);
}

TEST_CASE("ROM parser allocates CHR RAM when CHR ROM is absent") {
    std::vector<u8> data(16U + 16U * 1024U, 0);

    data[0] = 'N';
    data[1] = 'E';
    data[2] = 'S';
    data[3] = 0x1A;
    data[4] = 1;
    data[5] = 0;

    const Rom rom = Rom::parse(data);

    REQUIRE(rom.prg_rom.size() == 16U * 1024U);
    REQUIRE(rom.chr_data.size() == 8U * 1024U);
    REQUIRE(rom.chr_is_ram);
}

TEST_CASE("ROM parser rejects invalid header") {
    std::vector<u8> data(16U, 0);
    REQUIRE_THROWS_AS(Rom::parse(data), NesError);
}