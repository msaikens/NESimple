#pragma once

#include <memory>

#include "common/types.hpp"
#include "mappers/mapper.hpp"
#include "rom.hpp"

namespace nes {

class Cartridge {
public:
    explicit Cartridge(Rom rom);

    [[nodiscard]] u8 cpu_read(u16 addr) const;
    void cpu_write(u16 addr, u8 value);

    [[nodiscard]] u8 ppu_read(u16 addr) const;
    void ppu_write(u16 addr, u8 value);

    void tick_cpu(usize cpu_cycles);
    [[nodiscard]] bool irq_pending() const noexcept;
    void clear_irq() noexcept;

    [[nodiscard]] Mirroring mirroring() const noexcept;

private:
    std::unique_ptr<Mapper> mapper_;
    Mirroring mirroring_ {Mirroring::Horizontal};
};

} // namespace nes