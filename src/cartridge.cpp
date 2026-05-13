#include "cartridge.hpp"

#include <memory>
#include <string>
#include <utility>

#include "mappers/mapper.hpp"
#include "mappers/mapper0.hpp"
#include "mappers/mapper2.hpp"
#include "mappers/mapper23.hpp"

namespace nes {

Cartridge::Cartridge(Rom rom)
    : mirroring_(rom.mirroring) {
    switch (rom.mapper_id) {
    case 0:
        mapper_ = std::make_unique<Mapper0>(std::move(rom));
        break;

    case 2:
        mapper_ = std::make_unique<Mapper2>(std::move(rom));
        break;

    case 23:
        mapper_ = std::make_unique<Mapper23>(std::move(rom));
        break;

    default:
        throw NesError("Unsupported mapper: " + std::to_string(rom.mapper_id));
    }
}

u8 Cartridge::cpu_read(u16 addr) const {
    return mapper_->cpu_read(addr);
}

void Cartridge::cpu_write(u16 addr, u8 value) {
    mapper_->cpu_write(addr, value);
}

u8 Cartridge::ppu_read(u16 addr) const {
    return mapper_->ppu_read(addr);
}

void Cartridge::ppu_write(u16 addr, u8 value) {
    mapper_->ppu_write(addr, value);
}

void Cartridge::tick_cpu(usize cpu_cycles) {
    if (mapper_ != nullptr) {
        mapper_->tick_cpu(cpu_cycles);
    }
}

bool Cartridge::irq_pending() const noexcept {
    return mapper_ != nullptr && mapper_->irq_pending();
}

void Cartridge::clear_irq() noexcept {
    if (mapper_ != nullptr) {
        mapper_->clear_irq();
    }
}

Mirroring Cartridge::mirroring() const noexcept {
    if (mapper_ != nullptr && mapper_->has_dynamic_mirroring()) {
        return mapper_->mirroring();
    }

    return mirroring_;
}

} // namespace nes