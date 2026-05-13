// src/headers/common/types.hpp

// This header defines common types and utilities used throughout the NES emulator project.
// By centralizing these definitions, we ensure consistency and reduce boilerplate across the codebase.

// Author: Mitchell Aikens
// Date: 2024-06-01
// Time: 12:43 PM
// File Status: Completed

// C++ standard: 23
// Copyright 2026 Mitchell Aikens. All rights reserved.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace nes {
// Define fixed-width unsigned integer types for clarity and consistency.
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using usize = std::size_t;

// Custom exception type for NES-related errors.
class NesError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}