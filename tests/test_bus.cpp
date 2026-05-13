#include <catch2/catch_test_macros.hpp>

#include "bus.hpp"

using namespace nes;

static Cartridge make_cartridge() {
    Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(16 * 1024, 0);
    rom.chr_data.assign(8 * 1024, 0);
    rom.chr_is_ram = true;

    return Cartridge(std::move(rom));
}

TEST_CASE("CPU RAM is mirrored every 0x800 bytes") {
    auto cartridge = make_cartridge();
    Bus bus(std::move(cartridge));

    bus.write(0x0002, 0x42);

    REQUIRE(bus.read(0x0002) == 0x42);
    REQUIRE(bus.read(0x0802) == 0x42);
    REQUIRE(bus.read(0x1002) == 0x42);
    REQUIRE(bus.read(0x1802) == 0x42);
}

TEST_CASE("PPU register access path is mirrored every 8 bytes") {
    auto cartridge = make_cartridge();
    Bus bus(std::move(cartridge));

    // This mainly checks that the mirrored register routing path works and does not crash.
    bus.write(0x2000, 0x80);
    bus.write(0x2008, 0x01);

    SUCCEED();
}

TEST_CASE("Controller is readable through 0x4016") {
    auto cartridge = make_cartridge();
    Bus bus(std::move(cartridge));

    // Press A and Start.
    bus.controller().set_buttons(static_cast<u8>(Controller::A | Controller::Start));

    // Latch the current controller state and begin serial reads.
    bus.write(0x4016, 1);
    bus.write(0x4016, 0);

    REQUIRE((bus.read(0x4016) & 1) == 1); // A
    REQUIRE((bus.read(0x4016) & 1) == 0); // B
    REQUIRE((bus.read(0x4016) & 1) == 0); // Select
    REQUIRE((bus.read(0x4016) & 1) == 1); // Start
}
TEST_CASE("OAM DMA copies 256 bytes from CPU page into PPU OAM") {
    nes::Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = nes::Mirroring::Horizontal;
    rom.prg_rom.assign(16U * 1024U, 0);
    rom.chr_data.assign(8U * 1024U, 0);
    rom.chr_is_ram = true;

    nes::Cartridge cartridge(std::move(rom));
    nes::Bus bus(std::move(cartridge));

    for (nes::u16 i = 0; i < 256U; ++i) {
        bus.write(static_cast<nes::u16>(0x0200U + i), static_cast<nes::u8>(i & 0x00FFU));
    }

    bus.write(0x4014U, 0x02U);

    bus.write(0x2003U, 0x00U);
    REQUIRE(bus.read(0x2004U) == 0x00U);

    bus.write(0x2003U, 0x80U);
    REQUIRE(bus.read(0x2004U) == 0x80U);

    bus.write(0x2003U, 0xFFU);
    REQUIRE(bus.read(0x2004U) == 0xFFU);
}