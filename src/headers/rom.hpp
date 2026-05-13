#pragma once

#include <vector>

#include "common/types.hpp"

namespace nes {

enum class Mirroring {
    Horizontal,
    Vertical,
    OneScreenLower,
    OneScreenUpper
};

struct Rom {
    std::vector<u8> prg_rom;
    std::vector<u8> chr_data;
    u8 mapper_id {0};
    Mirroring mirroring {Mirroring::Horizontal};
    bool chr_is_ram {false};

    static Rom parse(const std::vector<u8>& data);
};

} // namespace nes