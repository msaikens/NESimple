#pragma once

#include "common/types.hpp"
#include "rom.hpp"

namespace nes {

class Mapper {
public:
    virtual ~Mapper() = default;

    virtual u8 cpu_read(u16 addr) const = 0;
    virtual void cpu_write(u16 addr, u8 value) = 0;

    virtual u8 ppu_read(u16 addr) const = 0;
    virtual void ppu_write(u16 addr, u8 value) = 0;

    virtual void tick_cpu(usize cpu_cycles) {
        static_cast<void>(cpu_cycles);
    }

    [[nodiscard]] virtual bool irq_pending() const noexcept {
        return false;
    }

    virtual void clear_irq() noexcept {
    }

    [[nodiscard]] virtual bool has_dynamic_mirroring() const noexcept {
        return false;
    }

    [[nodiscard]] virtual Mirroring mirroring() const noexcept {
        return Mirroring::Horizontal;
    }
};

} // namespace nes