#include "apu/apu_tables.hpp"

namespace nes::apu_tables {

const std::array<std::array<int, 8>, 4> kDutyTable {{
    {{0, 1, 0, 0, 0, 0, 0, 0}}, // 12.5%
    {{0, 1, 1, 0, 0, 0, 0, 0}}, // 25%
    {{0, 1, 1, 1, 1, 0, 0, 0}}, // 50%
    {{1, 0, 0, 1, 1, 1, 1, 1}}  // 25% negated
}};

const std::array<float, 32> kTriangleWave {
    15.0F, 14.0F, 13.0F, 12.0F, 11.0F, 10.0F, 9.0F, 8.0F,
    7.0F, 6.0F, 5.0F, 4.0F, 3.0F, 2.0F, 1.0F, 0.0F,
    0.0F, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F,
    8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F
};

const std::array<double, 16> kNoisePeriodsCpuCycles {
    4.0, 8.0, 16.0, 32.0,
    64.0, 96.0, 128.0, 160.0,
    202.0, 254.0, 380.0, 508.0,
    762.0, 1016.0, 2034.0, 4068.0
};

const std::array<u8, 32> kLengthTable {
    10U, 254U, 20U, 2U, 40U, 4U, 80U, 6U,
    160U, 8U, 60U, 10U, 14U, 12U, 26U, 14U,
    12U, 16U, 24U, 18U, 48U, 20U, 96U, 22U,
    192U, 24U, 72U, 26U, 16U, 28U, 32U, 30U
};

} // namespace nes::apu_tables