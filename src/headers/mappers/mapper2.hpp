#pragma once

#include <vector>
#include <array>

#include "mapper.hpp"
#include "rom.hpp"

namespace nes {

class Mapper2 final : public Mapper {
public:
    explicit Mapper2(Rom rom);

    u8 cpu_read(u16 addr) const override;
    void cpu_write(u16 addr, u8 value) override;

    u8 ppu_read(u16 addr) const override;
    void ppu_write(u16 addr, u8 value) override;

private:
    std::array<u8, 0x2000> prg_ram_ {};
    std::vector<u8> prg_rom_;
    std::vector<u8> chr_data_;
    bool chr_is_ram_ {false};
    usize selected_bank_ {0};
    usize prg_bank_count_ {0};
};

} // namespace nes