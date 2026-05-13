#pragma once

#include <array>

#include <cstdint>
#include "common/types.hpp"
#include "rom.hpp"

namespace nes {

class Cartridge;

class Ppu {
public:
    explicit Ppu(Mirroring mirroring = Mirroring::Horizontal) noexcept;
    void connect_cartridge(Cartridge* cartridge) noexcept;

    [[nodiscard]] u8 cpu_read(u16 addr);
    void cpu_write(u16 addr, u8 value);

    void tick(usize ppu_cycles);

    [[nodiscard]] bool poll_nmi() noexcept;
    [[nodiscard]] bool in_vblank() const noexcept;

    void oam_dma_write(u8 addr, u8 value) noexcept;

    void render_frame();

    [[nodiscard]] const std::array<u8, 256U * 240U>& frame_buffer() const noexcept;
    [[nodiscard]] u8 frame_pixel(usize x, usize y) const noexcept;
    [[nodiscard]] bool sprite_zero_hit() const noexcept;
    [[nodiscard]] uint64_t frame_counter() const noexcept;

    [[nodiscard]] uint64_t palette_write_count() const noexcept;
    [[nodiscard]] uint64_t nametable_write_count() const noexcept;
    [[nodiscard]] uint64_t pattern_write_count() const noexcept;
    [[nodiscard]] u8 mask() const noexcept;

private:
    void render_background();
    void render_background_scanline(usize y);
    void render_sprites();

    void increment_vram_addr() noexcept;

    [[nodiscard]] u8 read_ppu_data(u16 addr) const;
    void write_ppu_data(u16 addr, u8 value);

    [[nodiscard]] u16 mirror_ppu_addr(u16 addr) const noexcept;
    [[nodiscard]] u16 mirror_nametable_addr(u16 addr) const noexcept;
    [[nodiscard]] u16 palette_index(u16 addr) const noexcept;

    uint64_t palette_write_count_ {0};
    uint64_t nametable_write_count_ {0};
    uint64_t pattern_write_count_ {0};

    Cartridge* cartridge_ {nullptr};
    Mirroring mirroring_ {Mirroring::Horizontal};

    u8 open_bus_ {0};
    u8 ctrl_ {0};
    u8 mask_ {0};
    u8 status_ {0};
    u8 oam_addr_ {0};
    u8 fine_x_ {0};
    u16 vram_addr_ {0};
    u16 temp_vram_addr_ {0};
    bool write_toggle_ {false};
    u8 read_buffer_ {0};
    bool nmi_pending_ {false};

    u8 scroll_x_ {0};
    u8 scroll_y_ {0};

    uint64_t frame_counter_ {0};

    usize cycle_ {0};
    usize scanline_ {261};

    std::array<u8, 0x2000> pattern_table_ {};
    std::array<u8, 0x0800> vram_ {};
    std::array<u8, 0x0020> palette_ {};
    std::array<u8, 0x0100> oam_ {};
    std::array<u8, 256U * 240U> frame_buffer_ {};
};

} // namespace nes