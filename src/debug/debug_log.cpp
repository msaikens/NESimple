#include "debug/debug_log.hpp"

#include <iostream>
#include <mutex>

namespace nes {
namespace {

bool g_enabled = false;

std::uint32_t g_category_mask =
    debug_category_mask(DebugCategory::Cpu) |
    debug_category_mask(DebugCategory::Ppu) |
    debug_category_mask(DebugCategory::Apu) |
    debug_category_mask(DebugCategory::Mapper) |
    debug_category_mask(DebugCategory::Bus) |
    debug_category_mask(DebugCategory::Input) |
    debug_category_mask(DebugCategory::General);

std::mutex g_debug_mutex;

} // namespace

void DebugLog::set_enabled(bool enabled) noexcept {
    std::lock_guard lock(g_debug_mutex);
    g_enabled = enabled;
}

bool DebugLog::enabled() noexcept {
    std::lock_guard lock(g_debug_mutex);
    return g_enabled;
}

void DebugLog::enable_category(DebugCategory category) noexcept {
    std::lock_guard lock(g_debug_mutex);
    g_category_mask |= debug_category_mask(category);
}

void DebugLog::disable_category(DebugCategory category) noexcept {
    std::lock_guard lock(g_debug_mutex);
    g_category_mask &= ~debug_category_mask(category);
}

void DebugLog::set_categories(std::uint32_t mask) noexcept {
    std::lock_guard lock(g_debug_mutex);
    g_category_mask = mask;
}

bool DebugLog::category_enabled(DebugCategory category) noexcept {
    std::lock_guard lock(g_debug_mutex);
    return (g_category_mask & debug_category_mask(category)) != 0U;
}

void DebugLog::write(DebugCategory category, std::string_view message) {
    std::lock_guard lock(g_debug_mutex);

    if (!g_enabled || (g_category_mask & debug_category_mask(category)) == 0U) {
        return;
    }

    std::cerr << "[NESimple][" << category_name(category) << "] "
              << message
              << '\n';
}

const char* DebugLog::category_name(DebugCategory category) noexcept {
    switch (category) {
    case DebugCategory::Cpu:
        return "CPU";

    case DebugCategory::Ppu:
        return "PPU";

    case DebugCategory::Apu:
        return "APU";

    case DebugCategory::Mapper:
        return "MAPPER";

    case DebugCategory::Bus:
        return "BUS";

    case DebugCategory::Input:
        return "INPUT";

    case DebugCategory::General:
        return "GENERAL";
    }

    return "UNKNOWN";
}

} // namespace nes