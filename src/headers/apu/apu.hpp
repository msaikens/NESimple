#pragma once

#include <cstdint>
#include <vector>

#include "apu/envelope.hpp"
#include "apu/length_counter.hpp"
#include "apu/sweep.hpp"
#include "common/types.hpp"

namespace nes {

class Apu {
public:
    Apu();

    void reset();

    [[nodiscard]] u8 cpu_read(u16 addr);
    void cpu_write(u16 addr, u8 value);

    void tick(usize cpu_cycles);

    [[nodiscard]] std::vector<float> take_samples();

private:
    struct PulseChannel {
        bool enabled {false};
        bool is_pulse1 {false};
        u8 duty {0};
        u16 timer {0};
        double phase {0.0};

        Envelope envelope {};
        LengthCounter length_counter {};
        SweepUnit sweep {};

        void reset(bool pulse1) noexcept;
        void write_control(u8 value) noexcept;
        void write_sweep(u8 value) noexcept;
        void write_timer_low(u8 value) noexcept;
        void write_timer_high(u8 value) noexcept;
        void tick_half_frame() noexcept;

        [[nodiscard]] float sample(double cpu_frequency, double sample_rate) noexcept;
    };

    struct TriangleChannel {
        bool enabled {false};
        u16 timer {0};
        double phase {0.0};

        u8 linear_counter_reload {0};
        u8 linear_counter {0};
        bool control_flag {false};
        bool reload_flag {false};

        LengthCounter length_counter {};

        void reset() noexcept;
        void write_linear_control(u8 value) noexcept;
        void write_timer_low(u8 value) noexcept;
        void write_timer_high(u8 value) noexcept;
        void tick_quarter_frame() noexcept;

        [[nodiscard]] float sample(double cpu_frequency, double sample_rate) noexcept;
    };

    struct NoiseChannel {
        bool enabled {false};
        bool mode {false};
        u8 period_index {0};
        u16 shift_register {1};
        double phase {0.0};

        Envelope envelope {};
        LengthCounter length_counter {};

        void reset() noexcept;
        void write_control(u8 value) noexcept;
        void write_period(u8 value) noexcept;
        void write_length(u8 value) noexcept;

        [[nodiscard]] float sample(double cpu_frequency, double sample_rate) noexcept;
    };

    struct HighPassFilter {
        float previous_input {0.0F};
        float previous_output {0.0F};
        float coefficient {0.0F};

        void reset() noexcept;
        [[nodiscard]] float process(float input) noexcept;
    };

    struct LowPassFilter {
        float previous_output {0.0F};
        float coefficient {0.0F};

        void reset() noexcept;
        [[nodiscard]] float process(float input) noexcept;
    };

    void write_frame_counter(u8 value) noexcept;
    void tick_frame_counter(usize cpu_cycles) noexcept;

    void clock_quarter_frame() noexcept;
    void clock_half_frame() noexcept;

    void reset_filters() noexcept;
    [[nodiscard]] float apply_output_filters(float sample) noexcept;
    [[nodiscard]] float mix_sample() noexcept;

    PulseChannel pulse1_ {};
    PulseChannel pulse2_ {};
    TriangleChannel triangle_ {};
    NoiseChannel noise_ {};

    double sample_accumulator_ {0.0};

    std::uint64_t frame_counter_cycle_ {0};
    bool frame_counter_five_step_ {false};
    bool frame_irq_inhibit_ {false};
    bool frame_irq_pending_ {false};

    std::vector<float> sample_buffer_ {};

    HighPassFilter highpass_90hz_ {};
    HighPassFilter highpass_440hz_ {};
    LowPassFilter lowpass_14000hz_ {};

    static constexpr double kCpuFrequency = 1789773.0;
    static constexpr double kSampleRate = 48000.0;
    static constexpr double kCpuCyclesPerSample = kCpuFrequency / kSampleRate;
};

} // namespace nes