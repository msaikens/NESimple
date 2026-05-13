#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "debug/debug_log.hpp"
#include <SDL3/SDL_main.h>

#include "audio/audio_device.hpp"
#include "bus.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "input/input_config.hpp"
#include "rom.hpp"
#include "ui/app_window.hpp"

namespace {
constexpr bool kDebugTerminalOutput = true;

constexpr std::uint32_t kDebugCategories =
    nes::debug_category_mask(nes::DebugCategory::Cpu) |
//    nes::debug_category_mask(nes::DebugCategory::Ppu) |
    nes::debug_category_mask(nes::DebugCategory::Mapper);
//    nes::debug_category_mask(nes::DebugCategory::General);

std::vector<nes::u8> read_file_bytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw nes::NesError("Failed to open ROM file");
    }

    return {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };
}

bool has_flag(int argc, char** argv, const std::string& flag) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == flag) {
            return true;
        }
    }

    return false;
}

std::string get_option_value(int argc, char** argv, const std::string& option) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (argv[i] == option) {
            return argv[i + 1];
        }
    }

    return {};
}

std::string find_rom_path(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (!arg.empty() && arg[0] != '-') {
            if (i > 0 && std::string(argv[i - 1]) == "--trace-file") {
                continue;
            }

            if (i > 0 && std::string(argv[i - 1]) == "--steps") {
                continue;
            }

            if (i > 0 && std::string(argv[i - 1]) == "--trace-format") {
                continue;
            }

            return arg;
        }
    }

    return {};
}

int parse_steps(const std::string& value) {
    if (value.empty()) {
        return 0;
    }

    return std::stoi(value);
}

nes::Cpu::TraceFormat parse_trace_format(const std::string& value) {
    if (value == "nestest") {
        return nes::Cpu::TraceFormat::NestestLike;
    }

    return nes::Cpu::TraceFormat::Compact;
}

std::string rom_display_name(const std::string& rom_path) {
    return std::filesystem::path(rom_path).filename().string();
}

std::string build_window_title(
    const std::string& rom_name,
    bool paused,
    double fps,
    std::uint64_t frame_counter,
    bool show_frame_counter
) {
    std::ostringstream out;
    out << "NESimple"
        << " | " << rom_name
        << " | FPS " << std::fixed << std::setprecision(1) << fps;

    if (show_frame_counter) {
        out << " | FR " << frame_counter;
    }

    out << " | " << (paused ? "Paused" : "Running");

    return out.str();
}

struct Emulator {
    explicit Emulator(const nes::Rom& rom)
        : cartridge(rom)
        , bus(std::move(cartridge))
        , cpu(bus) {
        cpu.reset();
    }

    nes::Cartridge cartridge;
    nes::Bus bus;
    nes::Cpu cpu;
};

void run_until_next_frame(nes::Cpu& cpu) {
    const std::uint64_t frame_start = cpu.bus().ppu().frame_counter();

    std::uint64_t steps = 0;
    constexpr std::uint64_t warn_steps = 120000U;
    constexpr std::uint64_t max_steps = 500000U;

    std::array<nes::u16, 64> recent_pcs {};
    std::size_t recent_index = 0;

    bool warned = false;

    while (cpu.bus().ppu().frame_counter() == frame_start) {
        recent_pcs[recent_index % recent_pcs.size()] = cpu.pc();
        ++recent_index;

        cpu.step();
        ++steps;

        if (!warned && steps >= warn_steps) {
            warned = true;

            nes::DebugLog::write_if(nes::DebugCategory::Cpu, [&] {
                std::ostringstream out;
                out << "SLOW FRAME WARNING"
                    << " steps=" << steps
                    << " frame=" << frame_start
                    << " pc=$"
                    << std::hex << std::uppercase
                    << std::setw(4) << std::setfill('0')
                    << static_cast<int>(cpu.pc())
                    << " a=$" << std::setw(2) << static_cast<int>(cpu.a())
                    << " x=$" << std::setw(2) << static_cast<int>(cpu.x())
                    << " y=$" << std::setw(2) << static_cast<int>(cpu.y())
                    << " sp=$" << std::setw(2) << static_cast<int>(cpu.sp())
                    << " p=$" << std::setw(2) << static_cast<int>(cpu.status());
                return out.str();
            });
        }

        if (steps >= max_steps) {
            nes::DebugLog::write_if(nes::DebugCategory::Cpu, [&] {
                std::ostringstream out;

                out << "FRAME STALL DETECTED"
                    << " steps=" << steps
                    << " frame=" << frame_start
                    << " pc=$"
                    << std::hex << std::uppercase
                    << std::setw(4) << std::setfill('0')
                    << static_cast<int>(cpu.pc())
                    << " a=$" << std::setw(2) << static_cast<int>(cpu.a())
                    << " x=$" << std::setw(2) << static_cast<int>(cpu.x())
                    << " y=$" << std::setw(2) << static_cast<int>(cpu.y())
                    << " sp=$" << std::setw(2) << static_cast<int>(cpu.sp())
                    << " p=$" << std::setw(2) << static_cast<int>(cpu.status())
                    << " recent_pcs=";

                const std::size_t count =
                    recent_index < recent_pcs.size() ? recent_index : recent_pcs.size();

                const std::size_t start =
                    recent_index >= count ? recent_index - count : 0U;

                for (std::size_t i = 0; i < count; ++i) {
                    const auto pc_value =
                        recent_pcs[(start + i) % recent_pcs.size()];

                    out << "$"
                        << std::setw(4)
                        << static_cast<int>(pc_value);

                    if (i + 1U < count) {
                        out << ",";
                    }
                }

                return out.str();
            });

            break;
        }
    }
}

void push_audio_samples(nes::AudioDevice& audio_device, nes::Apu& apu) {
    if (!audio_device.available()) {
        (void)apu.take_samples();
        return;
    }

    audio_device.push_samples(apu.take_samples());
}

} // namespace

int main(int argc, char** argv) {
    try {
        nes::DebugLog::set_enabled(kDebugTerminalOutput);
        nes::DebugLog::set_categories(kDebugCategories);
        const std::string rom_path = find_rom_path(argc, argv);
        if (rom_path.empty()) {
            std::cerr
                << "Usage: NESimple <rom.nes> [--log-cpu] [--nestest] [--steps N] "
                << "[--trace-file path] [--trace-format compact|nestest] [--show-frame]\n";
            return 1;
        }

        const bool log_cpu = has_flag(argc, argv, "--log-cpu");
        const bool nestest_mode = has_flag(argc, argv, "--nestest");
        const bool show_frame = has_flag(argc, argv, "--show-frame");
        const int steps = parse_steps(get_option_value(argc, argv, "--steps"));
        const std::string trace_path = get_option_value(argc, argv, "--trace-file");
        const auto trace_format =
            parse_trace_format(get_option_value(argc, argv, "--trace-format"));

        const std::vector<nes::u8> bytes = read_file_bytes(rom_path);
        const nes::Rom rom = nes::Rom::parse(bytes);
        const std::string rom_name = rom_display_name(rom_path);

        auto emulator = std::make_unique<Emulator>(rom);

        if (nestest_mode) {
            emulator->cpu.set_pc(0xC000U);
        }

        std::unique_ptr<std::ofstream> trace_file;
        std::ostream* trace_stream = nullptr;

        if (!trace_path.empty()) {
            trace_file = std::make_unique<std::ofstream>(
                trace_path,
                std::ios::out | std::ios::trunc
            );

            if (!(*trace_file)) {
                throw nes::NesError("Failed to open trace output file");
            }

            trace_stream = trace_file.get();
        } else if (log_cpu) {
            trace_stream = &std::cout;
        }

        if (trace_stream != nullptr) {
            emulator->cpu.set_logging(true, trace_stream, trace_format);
        }

        if (!show_frame) {
            for (int i = 0; i < steps; ++i) {
                emulator->cpu.step();
            }

            return 0;
        }

        nes::AppWindow window;
        if (!window.available()) {
            std::cout << "SDL window unavailable in this build.\n";
            return 0;
        }

        {
            auto mapping = window.input_mapping();
            if (nes::load_input_mapping_from_file(
                    nes::default_input_mapping_path(),
                    mapping
                )) {
                window.set_input_mapping(mapping);
            }
        }

        nes::AudioDevice audio_device;

        bool paused = false;
        bool show_frame_counter = false;

        std::uint64_t frames_presented = 0;
        auto fps_window_start = std::chrono::steady_clock::now();
        double fps = 0.0;

        constexpr auto target_frame_time =
            std::chrono::duration<double>(1.0 / 60.0);

        while (window.process_events()) {
            const auto frame_start = std::chrono::steady_clock::now();

            if (window.consume_frame_counter_toggle_pressed()) {
                show_frame_counter = !show_frame_counter;
            }

            if (window.consume_pause_pressed()) {
                paused = !paused;
            }

            if (window.consume_reset_pressed()) {
                emulator = std::make_unique<Emulator>(rom);

                if (nestest_mode) {
                    emulator->cpu.set_pc(0xC000U);
                }

                if (trace_stream != nullptr) {
                    emulator->cpu.set_logging(true, trace_stream, trace_format);
                }

                paused = false;
            }

            const bool step_frame = window.consume_step_frame_pressed();
            const nes::u8 controller_state = window.poll_controller_state();
            emulator->cpu.bus().controller().set_buttons(controller_state);

            if (!paused || step_frame) {
                if (steps > 0) {
                    for (nes::usize i = 0; i < static_cast<nes::usize>(steps); ++i) {
                        emulator->cpu.step();
                    }
                } else {
                    run_until_next_frame(emulator->cpu);
                }

                push_audio_samples(audio_device, emulator->cpu.bus().apu());
            }

            const std::uint64_t current_frame =
                emulator->cpu.bus().ppu().frame_counter();

            nes::OverlayState overlay {};
            overlay.paused = paused;
            overlay.gamepad_connected = window.has_gamepad();
            overlay.fps = fps;
            overlay.frame_counter = show_frame_counter ? current_frame : 0U;
            overlay.pc = emulator->cpu.pc();
            overlay.controller_state = controller_state;
            overlay.rom_name = rom_name;

            window.present_frame(emulator->cpu.bus().ppu().frame_buffer(), overlay);
            window.set_title(
                build_window_title(
                    rom_name,
                    paused,
                    fps,
                    current_frame,
                    show_frame_counter
                )
            );

            ++frames_presented;
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<double> fps_elapsed = now - fps_window_start;

            if (fps_elapsed.count() >= 0.5) {
                fps = static_cast<double>(frames_presented) / fps_elapsed.count();
                frames_presented = 0;
                fps_window_start = now;
            }

            const auto frame_end = std::chrono::steady_clock::now();
            const std::chrono::duration<double> elapsed = frame_end - frame_start;

            if (elapsed < target_frame_time) {
                std::this_thread::sleep_for(target_frame_time - elapsed);
            }
        }

        nes::save_input_mapping_to_file(
            nes::default_input_mapping_path(),
            window.input_mapping()
        );

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}