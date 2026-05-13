// src/rom.cpp
// This source file implements the Rom structure and related functions for representing NES cartridge data.

// Author: Mitchell Aikens
// Date: May 6 2026
// Time: 1:11 PM ET
// File Status: Complete

// C++ standard: 20
// Copyright 2026 Mitchell Aikens. All rights reserved.


#include "rom.hpp"

namespace nes {
// Parse a ROM from raw iNES file data. Throws NesError on invalid format.
// This function performs a safe and thorough parsing of the iNES file format, validating the header,
// extracting the PRG and CHR data, and handling edge cases like missing CHR ROM (which implies CHR RAM).
// By centralizing all parsing logic here, we ensure that the rest of the emulator can rely on well-formed Rom objects.
Rom Rom::parse(const std::vector<u8>& data) {
// Validate the iNES header and extract metadata. This includes checking the magic number, determining
// the number of PRG and CHR banks, identifying the mapper ID, and determining the mirroring mode.
    if (data.size() < 16) {
        throw NesError("ROM too small for iNES header");
    }
// Validate the iNES header magic number.
// The first 4 bytes must be "NES" followed by 0x1A. This is a basic sanity check to ensure we're parsing a valid ROM file.
    if (!(data[0] == 'N' && data[1] == 'E' && data[2] == 'S' && data[3] == 0x1A)) {
        throw NesError("Invalid iNES header");
    }
// Extract metadata from the header. This includes the number of PRG and CHR banks, mapper ID, mirroring mode,
// and whether a trainer is present. We also calculate the expected sizes of the PRG and CHR data sections/
// based on the number of banks specified in the header.
    const usize prg_banks = data[4];
    const usize chr_banks = data[5];
    const u8 flags6 = data[6];
    const u8 flags7 = data[7];

    const bool has_trainer = (flags6 & 0x04U) != 0;
    const usize trainer_size = has_trainer ? 512U : 0U;

    const u8 mapper_low = static_cast<u8>(flags6 >> 4U);
    const u8 mapper_high = static_cast<u8>(flags7 & 0xF0U);
    const u8 mapper_id = static_cast<u8>(mapper_high | mapper_low);

    const Mirroring mirroring =
        (flags6 & 0x01U) != 0 ? Mirroring::Vertical : Mirroring::Horizontal;

    const usize prg_size = prg_banks * 16U * 1024U;
    const usize chr_size = chr_banks * 8U * 1024U;

    const usize prg_start = 16U + trainer_size;
    const usize prg_end = prg_start + prg_size;
    const usize chr_end = prg_end + chr_size;
// Validate that the file contains enough data for the specified PRG and CHR sizes. This ensures we don't
// read out of bounds when slicing the data into the Rom structure. If the file is truncated or malformed,
// we throw a NesError to indicate the problem.

    if (data.size() < prg_end) {
        throw NesError("ROM truncated before PRG data");
    }
// If there are CHR banks specified, validate that the file contains enough data for them as well. If CHR
// is absent (chr_banks == 0), we will allocate CHR RAM later, so we don't need to check for CHR data in that case.
// By performing these checks early, we ensure that the rest of the parsing logic can safely assume the data is well-formed.

    if (chr_banks > 0 && data.size() < chr_end) {
        throw NesError("ROM truncated before CHR data");
    }
// Construct the Rom object by slicing the input data into the appropriate sections for PRG ROM and CHR data.
// If CHR ROM is absent (chr_banks == 0), we allocate 8 KB of CHR RAM and set the chr_is_ram flag to true. This allows
// the rest of the emulator to treat CHR as RAM when necessary, while still supporting cartridges that include CHR ROM.
// By centralizing all parsing and validation logic in this function, we ensure that the rest of the emulator can rely on
// well-formed Rom objects without needing to worry about the details of the iNES file format
    Rom rom {};
    rom.mapper_id = mapper_id;
    rom.mirroring = mirroring;
// Copy the PRG ROM data from the input vector into the Rom structure. We use iterators to safely slice the data
// based on the calculated offsets. This ensures that we don't read out of bounds and that the
// PRG ROM is correctly extracted according to the iNES format specifications.
    rom.prg_rom.assign(
        data.begin() + static_cast<std::ptrdiff_t>(prg_start),
        data.begin() + static_cast<std::ptrdiff_t>(prg_end)
    );
// Handle the CHR data. If there are no CHR banks, we allocate 8 KB of CHR RAM and set the chr_is_ram flag.
// If CHR ROM is present, we slice it from the input data just like we did for PRG ROM. This allows us to support both
// types of cartridges (CHR ROM and CHR RAM) while keeping the parsing logic clean and centralized in one place.
    if (chr_banks == 0) {
        rom.chr_data.assign(8U * 1024U, 0);
        rom.chr_is_ram = true;
    } else {
        rom.chr_data.assign(
            data.begin() + static_cast<std::ptrdiff_t>(prg_end),
            data.begin() + static_cast<std::ptrdiff_t>(chr_end)
        );
        rom.chr_is_ram = false;
    }
// At this point, we have successfully parsed the iNES file data into a well-formed Rom object. We can return this object to the caller,
// which can then use it to initialize the cartridge and bus. By performing all parsing and validation in this function, 
// we ensure that the rest of the emulator can rely on the integrity of the Rom data without needing to worry about 
// the details of the iNES format.
    return rom;
}

}