#pragma once

#include <array>

#include "common/types.hpp"

namespace nes::apu_tables {

extern const std::array<std::array<int, 8>, 4> kDutyTable;
extern const std::array<float, 32> kTriangleWave;
extern const std::array<double, 16> kNoisePeriodsCpuCycles;
extern const std::array<u8, 32> kLengthTable;

} // namespace nes::apu_tables