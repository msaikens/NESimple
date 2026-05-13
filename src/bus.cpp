#include "bus.hpp"

#include <utility>

namespace nes {

Bus::Bus(Cartridge cartridge)
    : cartridge_(std::move(cartridge))
    , ppu_(cartridge_.mirroring()) {
    ppu_.connect_cartridge(&cartridge_);
}

u8 Bus::read(u16 addr) {
    if (addr <= 0x1FFFU) {
        return cpu_ram_[addr & 0x07FFU];
    }

    if (addr >= 0x2000U && addr <= 0x3FFFU) {
        const u16 reg = static_cast<u16>(0x2000U | (addr & 0x0007U));
        return ppu_.cpu_read(reg);
    }

    if (addr == 0x4015U) {
        return apu_.cpu_read(addr);
    }

    if (addr == 0x4016U) {
        return static_cast<u8>(controller1_.read() | 0x40U);
    }

    if (addr == 0x4017U) {
        return 0x40U;
    }

    if (addr >= 0x6000U) {
        return cartridge_.cpu_read(addr);
    }

    return 0;
}

void Bus::write(u16 addr, u8 value) {
    if (addr <= 0x1FFFU) {
        cpu_ram_[addr & 0x07FFU] = value;
        return;
    }

    if (addr >= 0x2000U && addr <= 0x3FFFU) {
        const u16 reg = static_cast<u16>(0x2000U | (addr & 0x0007U));
        ppu_.cpu_write(reg, value);
        return;
    }

    if (addr == 0x4014U) {
        const u16 base = static_cast<u16>(value) << 8U;

        for (u16 i = 0; i < 256U; ++i) {
            ppu_.oam_dma_write(
                static_cast<u8>(i),
                read(static_cast<u16>(base + i))
            );
        }

        return;
    }

    if (addr == 0x4016U) {
        controller1_.write(value);
        return;
    }

    if ((addr >= 0x4000U && addr <= 0x4013U) ||
        addr == 0x4015U ||
        addr == 0x4017U) {
        apu_.cpu_write(addr, value);
        return;
    }

    if (addr >= 0x6000U) {
        cartridge_.cpu_write(addr, value);
        return;
    }
}

Controller& Bus::controller() noexcept {
    return controller1_;
}

const Controller& Bus::controller() const noexcept {
    return controller1_;
}

Apu& Bus::apu() noexcept {
    return apu_;
}

const Apu& Bus::apu() const noexcept {
    return apu_;
}

void Bus::tick_cpu(usize cpu_cycles) {
    ppu_.tick(cpu_cycles * 3U);
    apu_.tick(cpu_cycles);
    cartridge_.tick_cpu(cpu_cycles);
}

bool Bus::poll_nmi() {
    return ppu_.poll_nmi();
}

bool Bus::poll_irq() {
    // Important:
    // Do NOT clear the mapper IRQ here.
    //
    // IRQ is level-like from the CPU's perspective. If the CPU interrupt-disable
    // flag is set, Cpu::irq() will ignore it for now. The mapper IRQ must remain
    // pending so the CPU can service it later when interrupts are enabled.
    //
    // Mapper23 clears its pending IRQ when the game writes the IRQ acknowledge
    // register, usually $F003. Clearing here can make games miss IRQs and hang
    // during timed transitions/cutscenes.
    return cartridge_.irq_pending();
}

Ppu& Bus::ppu() noexcept {
    return ppu_;
}

const Ppu& Bus::ppu() const noexcept {
    return ppu_;
}

} // namespace nes