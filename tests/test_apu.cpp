#include <catch2/catch_test_macros.hpp>

#include "apu/apu.hpp"
#include "bus.hpp"
#include "cartridge.hpp"
#include "rom.hpp"

using namespace nes;

namespace {

Bus make_bus() {
    Rom rom {};
    rom.mapper_id = 0;
    rom.mirroring = Mirroring::Horizontal;
    rom.prg_rom.assign(16U * 1024U, 0);
    rom.chr_data.assign(8U * 1024U, 0);
    rom.chr_is_ram = true;

    Cartridge cart(std::move(rom));
    return Bus(std::move(cart));
}

} // namespace

TEST_CASE("APU reset starts with pulse channels disabled") {
    Apu apu {};

    REQUIRE((apu.cpu_read(0x4015U) & 0x01U) == 0U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x02U) == 0U);
}

TEST_CASE("APU status reflects enabled pulse channels") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x01U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x01U) != 0U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x02U) == 0U);

    apu.cpu_write(0x4015U, 0x02U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x01U) == 0U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x02U) != 0U);

    apu.cpu_write(0x4015U, 0x03U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x01U) != 0U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x02U) != 0U);

    apu.cpu_write(0x4015U, 0x00U);
    REQUIRE((apu.cpu_read(0x4015U) & 0x03U) == 0U);
}

TEST_CASE("APU produces no samples before enough CPU cycles pass") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x01U);
    apu.cpu_write(0x4000U, 0x30U); // duty 0, constant volume 15
    apu.cpu_write(0x4002U, 0xFFU);
    apu.cpu_write(0x4003U, 0x07U);

    apu.tick(1U);

    const auto samples = apu.take_samples();
    REQUIRE(samples.empty());
}

TEST_CASE("APU produces samples after enough CPU cycles pass") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x01U);
    apu.cpu_write(0x4000U, 0x30U); // duty 0, constant volume 15
    apu.cpu_write(0x4002U, 0xFFU);
    apu.cpu_write(0x4003U, 0x07U);

    apu.tick(2000U);

    const auto samples = apu.take_samples();
    REQUIRE_FALSE(samples.empty());
}

TEST_CASE("APU take_samples drains the internal sample buffer") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x01U);
    apu.cpu_write(0x4000U, 0x30U);
    apu.cpu_write(0x4002U, 0xFFU);
    apu.cpu_write(0x4003U, 0x07U);

    apu.tick(2000U);

    const auto first = apu.take_samples();
    REQUIRE_FALSE(first.empty());

    const auto second = apu.take_samples();
    REQUIRE(second.empty());
}

TEST_CASE("APU disabled pulse channel produces silent samples") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x00U);
    apu.cpu_write(0x4000U, 0x30U);
    apu.cpu_write(0x4002U, 0xFFU);
    apu.cpu_write(0x4003U, 0x07U);

    apu.tick(2000U);

    const auto samples = apu.take_samples();
    REQUIRE_FALSE(samples.empty());

    for (const float sample : samples) {
        REQUIRE(sample == 0.0F);
    }
}

TEST_CASE("APU enabled pulse channel produces at least one non-silent sample") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x01U);
    apu.cpu_write(0x4000U, 0x3FU); // duty 0, constant volume, volume 15
    apu.cpu_write(0x4002U, 0x80U);
    apu.cpu_write(0x4003U, 0x02U);

    apu.tick(8000U);

    const auto samples = apu.take_samples();
    REQUIRE_FALSE(samples.empty());

    bool saw_nonzero = false;
    for (const float sample : samples) {
        if (sample != 0.0F) {
            saw_nonzero = true;
            break;
        }
    }

    REQUIRE(saw_nonzero);
}

TEST_CASE("Bus routes APU status writes and reads through 0x4015") {
    auto bus = make_bus();

    bus.write(0x4015U, 0x03U);

    REQUIRE((bus.read(0x4015U) & 0x01U) != 0U);
    REQUIRE((bus.read(0x4015U) & 0x02U) != 0U);

    bus.write(0x4015U, 0x00U);

    REQUIRE((bus.read(0x4015U) & 0x03U) == 0U);
}

TEST_CASE("Bus ticking CPU also ticks APU") {
    auto bus = make_bus();

    bus.write(0x4015U, 0x01U);
    bus.write(0x4000U, 0x30U);
    bus.write(0x4002U, 0x80U);
    bus.write(0x4003U, 0x02U);

    bus.tick_cpu(8000U);

    const auto samples = bus.apu().take_samples();
    REQUIRE_FALSE(samples.empty());
}
TEST_CASE("APU pulse sweep register does not prevent pulse sample generation") {
    Apu apu {};

    apu.cpu_write(0x4015U, 0x01U);
    apu.cpu_write(0x4000U, 0x3FU);
    apu.cpu_write(0x4001U, 0x08U); // sweep configured but disabled
    apu.cpu_write(0x4002U, 0x80U);
    apu.cpu_write(0x4003U, 0x20U);

    apu.tick(8000U);

    const auto samples = apu.take_samples();
    REQUIRE_FALSE(samples.empty());

    bool saw_nonzero = false;
    for (const float sample : samples) {
        if (sample != 0.0F) {
            saw_nonzero = true;
            break;
        }
    }

    REQUIRE(saw_nonzero);
}
TEST_CASE("APU frame counter write in 5-step mode does not break sample generation") {
    Apu apu {};

    apu.cpu_write(0x4017U, 0x80U);
    apu.cpu_write(0x4015U, 0x01U);
    apu.cpu_write(0x4000U, 0x3FU);
    apu.cpu_write(0x4002U, 0x80U);
    apu.cpu_write(0x4003U, 0x20U);

    apu.tick(8000U);

    const auto samples = apu.take_samples();
    REQUIRE_FALSE(samples.empty());

    bool saw_nonzero = false;
    for (const float sample : samples) {
        if (sample != 0.0F) {
            saw_nonzero = true;
            break;
        }
    }

    REQUIRE(saw_nonzero);
}