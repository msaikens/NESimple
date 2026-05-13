#pragma once

#include <array>

#include "apu/apu.hpp"
#include "cartridge.hpp"
#include "common/types.hpp"
#include "controller.hpp"
#include "ppu.hpp"

namespace nes {

class Bus {
public:
    explicit Bus(Cartridge cartridge);

    u8 read(u16 addr);
    void write(u16 addr, u8 value);

    Controller& controller() noexcept;
    const Controller& controller() const noexcept;

    Apu& apu() noexcept;
    const Apu& apu() const noexcept;

    void tick_cpu(usize cpu_cycles);
    [[nodiscard]] bool poll_nmi();
    [[nodiscard]] bool poll_irq();

    Ppu& ppu() noexcept;
    const Ppu& ppu() const noexcept;

    Bus(const Bus&) = delete;
    Bus& operator=(const Bus&) = delete;

    Bus(Bus&&) = delete;
    Bus& operator=(Bus&&) = delete;

private:
    std::array<u8, 2048> cpu_ram_ {};
    Cartridge cartridge_;
    Ppu ppu_;
    Apu apu_;
    Controller controller1_;
};

} // namespace nes