#include <catch2/catch_test_macros.hpp>

#include "mappers/mapper23.hpp"
#include "rom.hpp"

using namespace nes;

namespace {

Rom make_mapper23_rom() {
    Rom rom {};
    rom.mapper_id = 23;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(8U * 1024U * 8U, 0);
    rom.chr_data.assign(1U * 1024U * 16U, 0);
    rom.chr_is_ram = false;
    return rom;
}

} // namespace

TEST_CASE("Mapper23 IRQ latch low and high nibbles combine") {
    Mapper23 mapper(make_mapper23_rom());

    mapper.cpu_write(0xF000U, 0x0AU);
    mapper.cpu_write(0xF001U, 0x05U);
    mapper.cpu_write(0xF002U, 0x06U); // enable, cycle mode

    mapper.tick_cpu(0xA5U);

    REQUIRE(mapper.irq_pending());
}

TEST_CASE("Mapper23 IRQ acknowledge clears pending IRQ") {
    Mapper23 mapper(make_mapper23_rom());

    mapper.cpu_write(0xF000U, 0x0EU);
    mapper.cpu_write(0xF001U, 0x0FU);
    mapper.cpu_write(0xF002U, 0x06U); // enable, cycle mode

    mapper.tick_cpu(4U);

    REQUIRE(mapper.irq_pending());

    mapper.cpu_write(0xF003U, 0x00U);

    REQUIRE_FALSE(mapper.irq_pending());
}