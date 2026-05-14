#pragma once

#include <array>
#include <vector>

#include "mappers/mapper.hpp"
#include "rom.hpp"

namespace nes {

class Mapper9 final : public Mapper {
public:
    explicit Mapper9(Rom rom);

    u8 cpu_read(u16 addr) const override;
    void cpu_write(u16 addr, u8 value) override;

    u8 ppu_read(u16 addr) const override;
    void ppu_write(u16 addr, u8 value) override;

    [[nodiscard]] bool has_dynamic_mirroring() const noexcept override;
    [[nodiscard]] Mirroring mirroring() const noexcept override;

private:
    enum class LatchState : u8 {
        FD,
        FE
    };

    [[nodiscard]] usize prg_offset(usize bank, usize addr_offset) const noexcept;
    [[nodiscard]] usize chr_offset(usize bank, usize addr_offset) const noexcept;

    [[nodiscard]] usize selected_chr_bank_for_addr(u16 addr) const noexcept;
    void update_latches_for_ppu_read(u16 addr) const noexcept;

    std::vector<u8> prg_rom_ {};
    std::vector<u8> chr_data_ {};
    bool chr_is_ram_ {false};

    usize prg_bank_count_ {0};
    usize chr_bank_count_ {0};

    u8 prg_bank_8000_ {0};

    // CHR bank registers are 4 KB bank numbers.
    // $B000: PPU $0000-$0FFF when latch 0 = FD
    // $C000: PPU $0000-$0FFF when latch 0 = FE
    // $D000: PPU $1000-$1FFF when latch 1 = FD
    // $E000: PPU $1000-$1FFF when latch 1 = FE
    u8 chr_fd_0000_ {0};
    u8 chr_fe_0000_ {0};
    u8 chr_fd_1000_ {0};
    u8 chr_fe_1000_ {0};

    mutable LatchState latch_0000_ {LatchState::FD};
    mutable LatchState latch_1000_ {LatchState::FD};

    Mirroring mirroring_ {Mirroring::Horizontal};

    std::array<u8, 0x2000> prg_ram_ {};
};

} // namespace nes