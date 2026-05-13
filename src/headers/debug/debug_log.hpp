#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace nes {

enum class DebugCategory : std::uint32_t {
    Cpu     = 1U << 0U,
    Ppu     = 1U << 1U,
    Apu     = 1U << 2U,
    Mapper  = 1U << 3U,
    Bus     = 1U << 4U,
    Input   = 1U << 5U,
    General = 1U << 6U
};

class DebugLog {
public:
    DebugLog() = delete;

    static void set_enabled(bool enabled) noexcept;
    [[nodiscard]] static bool enabled() noexcept;

    static void enable_category(DebugCategory category) noexcept;
    static void disable_category(DebugCategory category) noexcept;
    static void set_categories(std::uint32_t mask) noexcept;

    [[nodiscard]] static bool category_enabled(DebugCategory category) noexcept;

    static void write(DebugCategory category, std::string_view message);

    template <typename Builder>
    static void write_if(DebugCategory category, Builder&& builder) {
        if (!enabled() || !category_enabled(category)) {
            return;
        }

        write(category, builder());
    }

private:
    static const char* category_name(DebugCategory category) noexcept;
};

[[nodiscard]] constexpr std::uint32_t debug_category_mask(DebugCategory category) noexcept {
    return static_cast<std::uint32_t>(category);
}

} // namespace nes