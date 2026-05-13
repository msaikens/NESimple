#include "ppu.hpp"

#include "cartridge.hpp"

namespace nes {

Ppu::Ppu(Mirroring mirroring) noexcept
    : mirroring_(mirroring) {
    oam_.fill(0xFFU);
}

void Ppu::connect_cartridge(Cartridge* cartridge) noexcept {
    cartridge_ = cartridge;
}

u8 Ppu::cpu_read(u16 addr) {
    switch (static_cast<u16>(0x2000U | (addr & 0x0007U))) {
    case 0x2002U: {
        const u8 value = static_cast<u8>((status_ & 0xE0U) | (open_bus_ & 0x1FU));

        status_ = static_cast<u8>(status_ & ~0x80U);
        write_toggle_ = false;
        open_bus_ = value;

        return value;
    }

    case 0x2004U:
        open_bus_ = oam_[oam_addr_];
        return open_bus_;

    case 0x2007U: {
        const u16 ppu_addr = mirror_ppu_addr(vram_addr_);
        const u8 value = read_ppu_data(ppu_addr);
        const bool palette_read = (ppu_addr & 0x3F00U) == 0x3F00U;

        increment_vram_addr();

        if (palette_read) {
            read_buffer_ = read_ppu_data(static_cast<u16>(ppu_addr - 0x1000U));
            open_bus_ = value;
            return value;
        }

        const u8 buffered = read_buffer_;
        read_buffer_ = value;
        open_bus_ = buffered;
        return buffered;
    }

    default:
        return open_bus_;
    }
}

void Ppu::cpu_write(u16 addr, u8 value) {
    open_bus_ = value;

    switch (static_cast<u16>(0x2000U | (addr & 0x0007U))) {
    case 0x2000U: {
        const bool old_nmi_enable = (ctrl_ & 0x80U) != 0U;

        ctrl_ = value;
        temp_vram_addr_ = static_cast<u16>(
            (temp_vram_addr_ & 0xF3FFU) | ((static_cast<u16>(value & 0x03U)) << 10U)
        );

        const bool new_nmi_enable = (ctrl_ & 0x80U) != 0U;

        if (!old_nmi_enable && new_nmi_enable && (status_ & 0x80U) != 0U) {
            nmi_pending_ = true;
        }

        break;
    }

    case 0x2001U:
        mask_ = value;
        break;

    case 0x2003U:
        oam_addr_ = value;
        break;

    case 0x2004U:
        oam_[oam_addr_] = value;
        oam_addr_ = static_cast<u8>(oam_addr_ + 1U);
        break;

    case 0x2005U:
        if (!write_toggle_) {
            fine_x_ = static_cast<u8>(value & 0x07U);
            scroll_x_ = value;
            temp_vram_addr_ = static_cast<u16>(
                (temp_vram_addr_ & 0x7FE0U) | (static_cast<u16>(value) >> 3U)
            );
            write_toggle_ = true;
        } else {
            scroll_y_ = value;
            temp_vram_addr_ = static_cast<u16>(
                (temp_vram_addr_ & 0x0C1FU)
                | ((static_cast<u16>(value & 0x07U)) << 12U)
                | ((static_cast<u16>(value & 0xF8U)) << 2U)
            );
            write_toggle_ = false;
        }
        break;

    case 0x2006U:
        if (!write_toggle_) {
            temp_vram_addr_ = static_cast<u16>(
                (temp_vram_addr_ & 0x00FFU) | ((static_cast<u16>(value & 0x3FU)) << 8U)
            );
            write_toggle_ = true;
        } else {
            temp_vram_addr_ = static_cast<u16>((temp_vram_addr_ & 0x7F00U) | value);
            vram_addr_ = static_cast<u16>(temp_vram_addr_ & 0x7FFFU);
            write_toggle_ = false;
        }
        break;

    case 0x2007U:
        write_ppu_data(vram_addr_, value);
        increment_vram_addr();
        break;

    default:
        break;
    }
}

void Ppu::tick(usize ppu_cycles) {
    for (usize i = 0; i < ppu_cycles; ++i) {
        ++cycle_;

        if (scanline_ < 240U && cycle_ == 256U) {
            render_background_scanline(scanline_);
        }

        if (cycle_ >= 341U) {
            cycle_ = 0U;
            ++scanline_;

            if (scanline_ >= 262U) {
                scanline_ = 0U;
                ++frame_counter_;
            }
        }

        if (scanline_ == 241U && cycle_ == 1U) {
            status_ = static_cast<u8>(status_ | 0x80U);

            if ((ctrl_ & 0x80U) != 0U) {
                nmi_pending_ = true;
            }

            // Background was already drawn scanline-by-scanline during visible
            // rendering. Overlay sprites at vblank so the presented frame is complete.
            status_ = static_cast<u8>(status_ & ~0x40U);
            render_sprites();
        }

        if (scanline_ == 261U && cycle_ == 2U) {
            status_ = static_cast<u8>(status_ & ~0x80U);
            status_ = static_cast<u8>(status_ & ~0x40U);
            write_toggle_ = false;
        }
    }
}

bool Ppu::poll_nmi() noexcept {
    if (!nmi_pending_) {
        return false;
    }

    nmi_pending_ = false;
    return true;
}

bool Ppu::in_vblank() const noexcept {
    return (status_ & 0x80U) != 0U;
}

void Ppu::oam_dma_write(u8 addr, u8 value) noexcept {
    oam_[addr] = value;
}

void Ppu::render_frame() {
    status_ = static_cast<u8>(status_ & ~0x40U);
    render_background();
    render_sprites();
}

void Ppu::render_background() {
    for (usize y = 0; y < 240U; ++y) {
        render_background_scanline(y);
    }
}

void Ppu::render_background_scanline(usize y) {
    const u16 bg_pattern_base = (ctrl_ & 0x10U) != 0U ? 0x1000U : 0x0000U;

    for (usize x = 0; x < 256U; ++x) {
        const usize scrolled_x = (x + static_cast<usize>(scroll_x_)) % 512U;
        const usize scrolled_y = (y + static_cast<usize>(scroll_y_)) % 480U;

        const usize tile_x = scrolled_x / 8U;
        const usize tile_y = scrolled_y / 8U;
        const usize col_in_tile = scrolled_x & 0x07U;
        const usize row_in_tile = scrolled_y & 0x07U;

        const u16 base_nametable = static_cast<u16>(ctrl_ & 0x03U);
        const u16 nametable_select = static_cast<u16>(
            (base_nametable + (tile_x / 32U) + ((tile_y / 30U) << 1U)) & 0x03U
        );
        const u16 nametable_base = static_cast<u16>(0x2000U + nametable_select * 0x0400U);

        const u16 local_tile_x = static_cast<u16>(tile_x % 32U);
        const u16 local_tile_y = static_cast<u16>(tile_y % 30U);

        const u16 nametable_addr = static_cast<u16>(
            nametable_base + local_tile_y * 32U + local_tile_x
        );
        const u8 tile_index = vram_[mirror_nametable_addr(nametable_addr)];

        const u16 attribute_addr = static_cast<u16>(
            nametable_base + 0x03C0U + (local_tile_y / 4U) * 8U + (local_tile_x / 4U)
        );
        const u8 attribute_byte = vram_[mirror_nametable_addr(attribute_addr)];

        const u8 quadrant_x = static_cast<u8>((local_tile_x % 4U) / 2U);
        const u8 quadrant_y = static_cast<u8>((local_tile_y % 4U) / 2U);
        const u8 shift = static_cast<u8>((quadrant_y * 2U + quadrant_x) * 2U);
        const u8 palette_select = static_cast<u8>((attribute_byte >> shift) & 0x03U);

        const u16 pattern_addr =
            static_cast<u16>(bg_pattern_base + tile_index * 16U + row_in_tile);
        const u8 low = read_ppu_data(pattern_addr);
        const u8 high = read_ppu_data(static_cast<u16>(pattern_addr + 8U));

        const u8 bit = static_cast<u8>(7U - col_in_tile);
        const u8 pixel_low = static_cast<u8>(
            ((low >> bit) & 0x01U) | (((high >> bit) & 0x01U) << 1U)
        );

        u8 palette_value = palette_[0];
        if (pixel_low != 0U) {
            const u8 palette_entry = static_cast<u8>(palette_select * 4U + pixel_low);
            palette_value = palette_[palette_entry];
        }

        frame_buffer_[y * 256U + x] = palette_value;
    }
}

void Ppu::render_sprites() {
    const bool sprite_8x16 = (ctrl_ & 0x20U) != 0U;
    const u16 sprite_pattern_base = (ctrl_ & 0x08U) != 0U ? 0x1000U : 0x0000U;
    const u8 backdrop = palette_[0];
    const usize sprite_height = sprite_8x16 ? 16U : 8U;

    for (int sprite = 63; sprite >= 0; --sprite) {
        const usize base = static_cast<usize>(sprite) * 4U;
        const u8 sprite_y = oam_[base + 0U];
        const u8 tile_index = oam_[base + 1U];
        const u8 attr = oam_[base + 2U];
        const u8 sprite_x = oam_[base + 3U];

        const bool priority_behind_bg = (attr & 0x20U) != 0U;
        const bool flip_horizontal = (attr & 0x40U) != 0U;
        const bool flip_vertical = (attr & 0x80U) != 0U;
        const u8 palette_select = static_cast<u8>(attr & 0x03U);

        const usize screen_y_base = static_cast<usize>(sprite_y) + 1U;
        if (screen_y_base >= 240U) {
            continue;
        }

        for (usize row = 0; row < sprite_height; ++row) {
            const usize screen_y = screen_y_base + row;
            if (screen_y >= 240U) {
                continue;
            }

            const usize effective_row = flip_vertical ? (sprite_height - 1U - row) : row;

            u16 pattern_addr = 0;
            if (sprite_8x16) {
                const u16 bank = static_cast<u16>((tile_index & 0x01U) * 0x1000U);
                const u16 tile_base = static_cast<u16>(tile_index & 0xFEU);
                const u16 tile_offset = static_cast<u16>(effective_row / 8U);
                const u16 row_in_tile = static_cast<u16>(effective_row % 8U);
                pattern_addr = static_cast<u16>(
                    bank + (tile_base + tile_offset) * 16U + row_in_tile
                );
            } else {
                pattern_addr = static_cast<u16>(
                    sprite_pattern_base + tile_index * 16U + effective_row
                );
            }

            const u8 low = read_ppu_data(pattern_addr);
            const u8 high = read_ppu_data(static_cast<u16>(pattern_addr + 8U));

            for (usize col = 0; col < 8U; ++col) {
                const usize screen_x = static_cast<usize>(sprite_x) + col;
                if (screen_x >= 256U) {
                    continue;
                }

                const usize bit_index = flip_horizontal ? col : (7U - col);
                const u8 pixel_low = static_cast<u8>(
                    ((low >> bit_index) & 0x01U) | (((high >> bit_index) & 0x01U) << 1U)
                );

                if (pixel_low == 0U) {
                    continue;
                }

                const usize index = screen_y * 256U + screen_x;
                const bool bg_is_opaque = frame_buffer_[index] != backdrop;

                if (sprite == 0 && bg_is_opaque && screen_x != 255U) {
                    status_ = static_cast<u8>(status_ | 0x40U);
                }

                if (priority_behind_bg && bg_is_opaque) {
                    continue;
                }

                const u8 palette_entry =
                    static_cast<u8>(0x10U + palette_select * 4U + pixel_low);
                frame_buffer_[index] =
                    palette_[palette_index(static_cast<u16>(0x3F00U + palette_entry))];
            }
        }
    }
}

const std::array<u8, 256U * 240U>& Ppu::frame_buffer() const noexcept {
    return frame_buffer_;
}

u8 Ppu::frame_pixel(usize x, usize y) const noexcept {
    return frame_buffer_[y * 256U + x];
}

bool Ppu::sprite_zero_hit() const noexcept {
    return (status_ & 0x40U) != 0U;
}

u8 Ppu::read_ppu_data(u16 addr) const {
    addr = mirror_ppu_addr(addr);

    if (addr < 0x2000U) {
        if (cartridge_ != nullptr) {
            return cartridge_->ppu_read(addr);
        }

        return pattern_table_[addr];
    }

    if (addr < 0x3F00U) {
        return vram_[mirror_nametable_addr(addr)];
    }

    return palette_[palette_index(addr)];
}

void Ppu::write_ppu_data(u16 addr, u8 value) {
    addr = mirror_ppu_addr(addr);

    if (addr < 0x2000U) {
        ++pattern_write_count_;

        if (cartridge_ != nullptr) {
            cartridge_->ppu_write(addr, value);
            return;
        }

        pattern_table_[addr] = value;
        return;
    }

    if (addr < 0x3F00U) {
        ++nametable_write_count_;
        vram_[mirror_nametable_addr(addr)] = value;
        return;
    }

    ++palette_write_count_;
    palette_[palette_index(addr)] = value;
}

u16 Ppu::mirror_ppu_addr(u16 addr) const noexcept {
    addr &= 0x3FFFU;

    if (addr >= 0x3000U && addr < 0x3F00U) {
        addr = static_cast<u16>(addr - 0x1000U);
    }

    return addr;
}

u16 Ppu::mirror_nametable_addr(u16 addr) const noexcept {
    const u16 offset = static_cast<u16>((addr - 0x2000U) & 0x0FFFU);
    const u16 table = static_cast<u16>(offset / 0x0400U);
    const u16 inner = static_cast<u16>(offset & 0x03FFU);

    const Mirroring current_mirroring =
        cartridge_ != nullptr ? cartridge_->mirroring() : mirroring_;

    u16 physical = 0;

    switch (current_mirroring) {
    case Mirroring::Vertical:
        physical = static_cast<u16>(table & 0x0001U);
        break;

    case Mirroring::Horizontal:
        physical = static_cast<u16>((table >> 1U) & 0x0001U);
        break;

    case Mirroring::OneScreenLower:
        physical = 0U;
        break;

    case Mirroring::OneScreenUpper:
        physical = 1U;
        break;
    }

    return static_cast<u16>(physical * 0x0400U + inner);
}

u16 Ppu::palette_index(u16 addr) const noexcept {
    u16 index = static_cast<u16>(addr & 0x001FU);

    if (index == 0x0010U) {
        index = 0x0000U;
    } else if (index == 0x0014U) {
        index = 0x0004U;
    } else if (index == 0x0018U) {
        index = 0x0008U;
    } else if (index == 0x001CU) {
        index = 0x000CU;
    }

    return index;
}

void Ppu::increment_vram_addr() noexcept {
    vram_addr_ = static_cast<u16>(
        (vram_addr_ + ((ctrl_ & 0x04U) != 0U ? 32U : 1U)) & 0x7FFFU
    );
}

uint64_t Ppu::frame_counter() const noexcept {
    return frame_counter_;
}

std::uint64_t Ppu::palette_write_count() const noexcept {
    return palette_write_count_;
}

std::uint64_t Ppu::nametable_write_count() const noexcept {
    return nametable_write_count_;
}

std::uint64_t Ppu::pattern_write_count() const noexcept {
    return pattern_write_count_;
}

u8 Ppu::mask() const noexcept {
    return mask_;
}

} // namespace nes