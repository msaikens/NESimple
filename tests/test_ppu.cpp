#include <catch2/catch_test_macros.hpp>

#include "ppu.hpp"
#include "cartridge.hpp"
#include "bus.hpp"


using namespace nes;

namespace {

constexpr usize kTicksToVblank = 242U * 341U + 1U;

void set_ppu_addr(Ppu& ppu, u16 addr) {
    ppu.cpu_write(0x2006U, static_cast<u8>((addr >> 8U) & 0x3FU));
    ppu.cpu_write(0x2006U, static_cast<u8>(addr & 0x00FFU));
}

void write_ppu_data(Ppu& ppu, u16 addr, u8 value) {
    set_ppu_addr(ppu, addr);
    ppu.cpu_write(0x2007U, value);
}

u8 read_ppu_data_with_dummy(Ppu& ppu, u16 addr) {
    set_ppu_addr(ppu, addr);
    static_cast<void>(ppu.cpu_read(0x2007U));
    return ppu.cpu_read(0x2007U);
}

void write_oam_sprite(Ppu& ppu, u8 index, u8 y, u8 tile, u8 attr, u8 x) {
    ppu.cpu_write(0x2003U, static_cast<u8>(index * 4U));
    ppu.cpu_write(0x2004U, y);
    ppu.cpu_write(0x2004U, tile);
    ppu.cpu_write(0x2004U, attr);
    ppu.cpu_write(0x2004U, x);
}

} // namespace

TEST_CASE("PPU status read clears vblank and address latch") {
    Ppu ppu;

    ppu.tick(kTicksToVblank);
    REQUIRE(ppu.in_vblank());

    set_ppu_addr(ppu, 0x2100U);

    const u8 status = ppu.cpu_read(0x2002U);
    REQUIRE((status & 0x80U) == 0x80U);
    REQUIRE_FALSE(ppu.in_vblank());

    write_ppu_data(ppu, 0x2300U, 0x5AU);
    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2300U) == 0x5AU);
}

TEST_CASE("PPU data port increments by 1 or 32 based on control") {
    Ppu ppu;

    ppu.cpu_write(0x2000U, 0x00U);
    write_ppu_data(ppu, 0x2000U, 0x11U);
    write_ppu_data(ppu, 0x2001U, 0x22U);

    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2000U) == 0x11U);
    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2001U) == 0x22U);

    ppu.cpu_write(0x2000U, 0x04U);
    set_ppu_addr(ppu, 0x2100U);
    ppu.cpu_write(0x2007U, 0x33U);
    ppu.cpu_write(0x2007U, 0x44U);

    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2100U) == 0x33U);
    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2120U) == 0x44U);
}

TEST_CASE("PPU enters vblank and raises NMI when enabled") {
    Ppu ppu;

    ppu.cpu_write(0x2000U, 0x80U);
    ppu.tick(kTicksToVblank);

    REQUIRE(ppu.in_vblank());
    REQUIRE(ppu.poll_nmi());
    REQUIRE_FALSE(ppu.poll_nmi());
}

TEST_CASE("Enabling NMI during vblank raises pending NMI") {
    Ppu ppu;

    ppu.tick(kTicksToVblank);
    REQUIRE(ppu.in_vblank());

    ppu.cpu_write(0x2000U, 0x80U);
    REQUIRE(ppu.poll_nmi());
}

TEST_CASE("PPU vertical mirroring maps 2000 and 2800 together") {
    Ppu ppu(Mirroring::Vertical);

    write_ppu_data(ppu, 0x2000U, 0x12U);
    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2800U) == 0x12U);
}

TEST_CASE("PPU horizontal mirroring maps 2000 and 2400 together") {
    Ppu ppu(Mirroring::Horizontal);

    write_ppu_data(ppu, 0x2000U, 0x34U);
    REQUIRE(read_ppu_data_with_dummy(ppu, 0x2400U) == 0x34U);
}

TEST_CASE("PPU background renderer draws tile pattern into framebuffer") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x11U);
    write_ppu_data(ppu, 0x3F02U, 0x22U);
    write_ppu_data(ppu, 0x3F03U, 0x33U);

    write_ppu_data(ppu, 0x0010U, 0x80U);
    write_ppu_data(ppu, 0x0018U, 0x00U);

    write_ppu_data(ppu, 0x2000U, 0x01U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 0U) == 0x11U);
    REQUIRE(ppu.frame_pixel(1U, 0U) == 0x00U);
}

TEST_CASE("PPU attribute table selects background palette") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F09U, 0x66U);

    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);

    write_ppu_data(ppu, 0x2000U, 0x00U);
    write_ppu_data(ppu, 0x23C0U, 0x02U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 0U) == 0x66U);
}

TEST_CASE("PPU background renderer uses second tile row data") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x44U);

    write_ppu_data(ppu, 0x0021U, 0x40U);
    write_ppu_data(ppu, 0x0029U, 0x00U);

    write_ppu_data(ppu, 0x2001U, 0x02U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(9U, 1U) == 0x44U);
    REQUIRE(ppu.frame_pixel(8U, 1U) == 0x00U);
}

TEST_CASE("PPU sprite renderer draws sprite pixels") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 1U) == 0x77U);
    REQUIRE(ppu.frame_pixel(1U, 1U) == 0x00U);
}

TEST_CASE("PPU sprite priority behind background is respected") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x11U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);
    write_ppu_data(ppu, 0x2000U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0x20U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 0U) == 0x11U);
    REQUIRE(ppu.frame_pixel(0U, 1U) == 0x77U);
}

TEST_CASE("PPU sprite horizontal flip is respected") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0x40U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(7U, 1U) == 0x77U);
    REQUIRE(ppu.frame_pixel(0U, 1U) == 0x00U);
}
TEST_CASE("PPU sprite vertical flip is respected") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    write_ppu_data(ppu, 0x0000U, 0x00U);
    write_ppu_data(ppu, 0x0007U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);
    write_ppu_data(ppu, 0x000FU, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0x80U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 1U) == 0x77U);
    REQUIRE(ppu.frame_pixel(0U, 8U) == 0x00U);
}
TEST_CASE("PPU lower OAM index sprite appears in front of higher index sprite") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x55U);
    write_ppu_data(ppu, 0x3F15U, 0x99U);

    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);
    write_ppu_data(ppu, 0x0010U, 0x80U);
    write_ppu_data(ppu, 0x0018U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0x00U, 0U);
    write_oam_sprite(ppu, 1U, 0U, 1U, 0x01U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 1U) == 0x55U);
}
TEST_CASE("PPU sets sprite zero hit when sprite zero overlaps opaque background") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x11U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    // Tile 0 row 0: opaque for sprite pixel at screen y = 1.
    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);

    // Tile 0 row 1: opaque for background pixel at screen y = 1.
    write_ppu_data(ppu, 0x0001U, 0x80U);
    write_ppu_data(ppu, 0x0009U, 0x00U);

    write_ppu_data(ppu, 0x2000U, 0x00U);
    write_oam_sprite(ppu, 0U, 0U, 0U, 0x00U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.sprite_zero_hit());
}
TEST_CASE("PPU does not set sprite zero hit on backdrop-only background") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    write_ppu_data(ppu, 0x0000U, 0x80U);
    write_ppu_data(ppu, 0x0008U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0x00U, 0U);

    ppu.render_frame();

    REQUIRE_FALSE(ppu.sprite_zero_hit());
}
TEST_CASE("PPU frame counter increments at vblank") {
    Ppu ppu;

    REQUIRE(ppu.frame_counter() == 0U);

    ppu.tick(kTicksToVblank);

    REQUIRE(ppu.frame_counter() == 1U);
}
TEST_CASE("PPU horizontal scroll shifts background fetch") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x22U);

    write_ppu_data(ppu, 0x0010U, 0x80U);
    write_ppu_data(ppu, 0x0018U, 0x00U);

    write_ppu_data(ppu, 0x2001U, 0x01U);

    ppu.cpu_write(0x2005U, 0x08U);
    ppu.cpu_write(0x2005U, 0x00U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 0U) == 0x22U);
}
TEST_CASE("PPU 8x16 sprite mode uses second tile for lower half") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    ppu.cpu_write(0x2000U, 0x20U);

    write_ppu_data(ppu, 0x1000U, 0x00U);
    write_ppu_data(ppu, 0x1008U, 0x00U);

    write_ppu_data(ppu, 0x1010U, 0x80U);
    write_ppu_data(ppu, 0x1018U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0x01U, 0x00U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 1U) == 0x00U);
    REQUIRE(ppu.frame_pixel(0U, 9U) == 0x77U);
}
TEST_CASE("PPU vertical scroll wraps into lower nametable") {
    Ppu ppu(Mirroring::Horizontal);

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x33U);

    write_ppu_data(ppu, 0x0010U, 0x80U);
    write_ppu_data(ppu, 0x0018U, 0x00U);

    write_ppu_data(ppu, 0x2800U, 0x01U);

    ppu.cpu_write(0x2005U, 0x00U);
    ppu.cpu_write(0x2005U, 0xF0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 0U) == 0x33U);
}
TEST_CASE("PPU combined scroll wraps into bottom right nametable") {
    Ppu ppu(Mirroring::Vertical);

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x44U);

    write_ppu_data(ppu, 0x0010U, 0x80U);
    write_ppu_data(ppu, 0x0018U, 0x00U);

    write_ppu_data(ppu, 0x2C00U, 0x01U);

    ppu.cpu_write(0x2005U, 0xF8U);
    ppu.cpu_write(0x2005U, 0xF0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(8U, 0U) == 0x44U);
}
TEST_CASE("PPU 8x16 sprite vertical flip uses upper tile on bottom half") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    ppu.cpu_write(0x2000U, 0x20U);

    write_ppu_data(ppu, 0x1007U, 0x80U);
    write_ppu_data(ppu, 0x100FU, 0x00U);

    write_ppu_data(ppu, 0x1010U, 0x00U);
    write_ppu_data(ppu, 0x1018U, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0x01U, 0x80U, 0U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 9U) == 0x77U);
}
TEST_CASE("PPU background renderer honors control base nametable") {
    Ppu ppu(Mirroring::Vertical);

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x55U);

    write_ppu_data(ppu, 0x0010U, 0x80U);
    write_ppu_data(ppu, 0x0018U, 0x00U);

    write_ppu_data(ppu, 0x2400U, 0x01U);

    ppu.cpu_write(0x2000U, 0x01U);

    ppu.render_frame();

    REQUIRE(ppu.frame_pixel(0U, 0U) == 0x55U);
}
TEST_CASE("PPU sprite zero hit is not set at x 255") {
    Ppu ppu;

    write_ppu_data(ppu, 0x3F00U, 0x00U);
    write_ppu_data(ppu, 0x3F01U, 0x11U);
    write_ppu_data(ppu, 0x3F11U, 0x77U);

    write_ppu_data(ppu, 0x0000U, 0x01U);
    write_ppu_data(ppu, 0x0008U, 0x00U);

    write_ppu_data(ppu, 0x0001U, 0x01U);
    write_ppu_data(ppu, 0x0009U, 0x00U);

    write_ppu_data(ppu, 0x201FU, 0x00U);

    write_oam_sprite(ppu, 0U, 0U, 0U, 0x00U, 248U);

    ppu.render_frame();

    REQUIRE_FALSE(ppu.sprite_zero_hit());
    REQUIRE(ppu.frame_pixel(255U, 1U) == 0x77U);
}
TEST_CASE("CPU-visible PPUSTATUS eventually reports vblank through bus timing") {
    Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(32U * 1024U, 0xEAU);
    rom.chr_data.assign(8U * 1024U, 0x00U);
    rom.chr_is_ram = false;

    Cartridge cartridge(std::move(rom));
    Bus bus(std::move(cartridge));

    bool saw_vblank = false;

    for (usize i = 0; i < 30000U; ++i) {
        bus.tick_cpu(1U);

        const u8 status = bus.read(0x2002U);
        if ((status & 0x80U) != 0U) {
            saw_vblank = true;
            break;
        }
    }

    REQUIRE(saw_vblank);
}