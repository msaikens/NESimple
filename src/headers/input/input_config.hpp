#pragma once

#include <filesystem>

#include "input/input_mapping.hpp"

namespace nes {

[[nodiscard]] std::filesystem::path default_userdata_dir();
[[nodiscard]] std::filesystem::path default_input_mapping_path();

[[nodiscard]] bool load_input_mapping_from_file(
    const std::filesystem::path& path,
    InputMapping& mapping
);

bool save_input_mapping_to_file(
    const std::filesystem::path& path,
    const InputMapping& mapping
);

} // namespace nes