#include "apu/apu.hpp"

#include "apu/apu_tables.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace nes {
namespace {

constexpr std::uint64_t kFrameStep1 = 7457ULL;
constexpr std::uint64_t kFrameStep2 = 14913ULL;
constexpr std::uint64_t kFrameStep3 = 22371ULL;
constexpr std::uint64_t kFrameStep4 = 29829ULL;
constexpr std::uint64_t kFrameStep5 = 37281ULL;

float highpass_coefficient(float cutoff_hz, float sample_rate) noexcept {
    constexpr float pi = 3.14159265358979323846F;
    const float rc = 1.0F / (2.0F * pi * cutoff_hz);
    const float dt = 1.0F / sample_rate;
    return rc / (rc + dt);
}

float lowpass_coefficient(float cutoff_hz, float sample_rate) noexcept {
    constexpr float pi = 3.14159265358979323846F;
    const float rc = 1.0F / (2.0F * pi * cutoff_hz);
    const float dt = 1.0F / sample_rate;
    return dt / (rc + dt);
}

} // namespace

Apu::Apu() {
    highpass_90hz_.coefficient = highpass_coefficient(90.0F, static_cast<float>(kSampleRate));
    highpass_440hz_.coefficient = highpass_coefficient(440.0F, static_cast<float>(kSampleRate));
    lowpass_14000hz_.coefficient = lowpass_coefficient(14000.0F, static_cast<float>(kSampleRate));

    reset();
    sample_buffer_.reserve(4096);
}

void Apu::reset() {
    pulse1_.reset(true);
    pulse2_.reset(false);
    triangle_.reset();
    noise_.reset();

    sample_accumulator_ = 0.0;

    frame_counter_cycle_ = 0;
    frame_counter_five_step_ = false;
    frame_irq_inhibit_ = false;
    frame_irq_pending_ = false;

    sample_buffer_.clear();
    reset_filters();
}

void Apu::HighPassFilter::reset() noexcept {
    previous_input = 0.0F;
    previous_output = 0.0F;
}

float Apu::HighPassFilter::process(float input) noexcept {
    const float output = coefficient * (previous_output + input - previous_input);
    previous_input = input;
    previous_output = output;
    return output;
}

void Apu::LowPassFilter::reset() noexcept {
    previous_output = 0.0F;
}

float Apu::LowPassFilter::process(float input) noexcept {
    previous_output += coefficient * (input - previous_output);
    return previous_output;
}

void Apu::reset_filters() noexcept {
    highpass_90hz_.reset();
    highpass_440hz_.reset();
    lowpass_14000hz_.reset();
}

float Apu::apply_output_filters(float sample) noexcept {
    float output = highpass_90hz_.process(sample);
    output = highpass_440hz_.process(output);
    output = lowpass_14000hz_.process(output);

    // The nonlinear mixer output is fairly quiet after filtering.
    output *= 3.2F;

    return std::clamp(output, -1.0F, 1.0F);
}

void Apu::PulseChannel::reset(bool pulse1) noexcept {
    enabled = false;
    is_pulse1 = pulse1;
    duty = 0;
    timer = 0;
    phase = 0.0;
    envelope.reset();
    length_counter.reset();
    sweep.reset();
}

void Apu::PulseChannel::write_control(u8 value) noexcept {
    duty = static_cast<u8>((value >> 6U) & 0x03U);
    envelope.write_control(value);
}

void Apu::PulseChannel::write_sweep(u8 value) noexcept {
    sweep.write(value);
}

void Apu::PulseChannel::write_timer_low(u8 value) noexcept {
    timer = static_cast<u16>((timer & 0x0700U) | value);
}

void Apu::PulseChannel::write_timer_high(u8 value) noexcept {
    timer = static_cast<u16>(
        (timer & 0x00FFU) | ((static_cast<u16>(value & 0x07U)) << 8U)
    );

    length_counter.load(static_cast<u8>((value >> 3U) & 0x1FU));
    envelope.restart();
    sweep.restart();
    phase = 0.0;
}

void Apu::PulseChannel::tick_half_frame() noexcept {
    length_counter.tick_half_frame(envelope.loop_flag());
    sweep.tick_half_frame(timer, is_pulse1);
}

float Apu::PulseChannel::sample(double cpu_frequency, double sample_rate) noexcept {
    if (!enabled || !length_counter.active() || timer < 8U || sweep.muted(timer)) {
        return 0.0F;
    }

    const u8 volume = envelope.output_volume();
    if (volume == 0U) {
        return 0.0F;
    }

    const double frequency = cpu_frequency / (16.0 * static_cast<double>(timer + 1U));
    if (frequency <= 0.0 || frequency > 20000.0) {
        return 0.0F;
    }

    phase += frequency / sample_rate;
    while (phase >= 1.0) {
        phase -= 1.0;
    }

    const int duty_step = static_cast<int>(phase * 8.0) & 0x07;
    const int high =
        apu_tables::kDutyTable[duty & 0x03U][static_cast<std::size_t>(duty_step)];

    return high != 0 ? static_cast<float>(volume) : 0.0F;
}

void Apu::TriangleChannel::reset() noexcept {
    enabled = false;
    timer = 0;
    phase = 0.0;
    linear_counter_reload = 0;
    linear_counter = 0;
    control_flag = false;
    reload_flag = false;
    length_counter.reset();
}

void Apu::TriangleChannel::write_linear_control(u8 value) noexcept {
    control_flag = (value & 0x80U) != 0U;
    linear_counter_reload = static_cast<u8>(value & 0x7FU);
}

void Apu::TriangleChannel::write_timer_low(u8 value) noexcept {
    timer = static_cast<u16>((timer & 0x0700U) | value);
}

void Apu::TriangleChannel::write_timer_high(u8 value) noexcept {
    timer = static_cast<u16>(
        (timer & 0x00FFU) | ((static_cast<u16>(value & 0x07U)) << 8U)
    );

    length_counter.load(static_cast<u8>((value >> 3U) & 0x1FU));
    reload_flag = true;
    phase = 0.0;
}

void Apu::TriangleChannel::tick_quarter_frame() noexcept {
    if (reload_flag) {
        linear_counter = linear_counter_reload;
    } else if (linear_counter > 0U) {
        --linear_counter;
    }

    if (!control_flag) {
        reload_flag = false;
    }
}

float Apu::TriangleChannel::sample(double cpu_frequency, double sample_rate) noexcept {
    if (!enabled ||
        !length_counter.active() ||
        linear_counter == 0U ||
        timer < 2U) {
        return 0.0F;
    }

    const double frequency = cpu_frequency / (32.0 * static_cast<double>(timer + 1U));
    if (frequency <= 0.0 || frequency > 20000.0) {
        return 0.0F;
    }

    phase += frequency / sample_rate;
    while (phase >= 1.0) {
        phase -= 1.0;
    }

    const int step = static_cast<int>(phase * 32.0) & 0x1F;
    return apu_tables::kTriangleWave[static_cast<std::size_t>(step)];
}

void Apu::NoiseChannel::reset() noexcept {
    enabled = false;
    mode = false;
    period_index = 0;
    shift_register = 1U;
    phase = 0.0;
    envelope.reset();
    length_counter.reset();
}

void Apu::NoiseChannel::write_control(u8 value) noexcept {
    envelope.write_control(value);
}

void Apu::NoiseChannel::write_period(u8 value) noexcept {
    mode = (value & 0x80U) != 0U;
    period_index = static_cast<u8>(value & 0x0FU);
}

void Apu::NoiseChannel::write_length(u8 value) noexcept {
    length_counter.load(static_cast<u8>((value >> 3U) & 0x1FU));
    envelope.restart();
    shift_register = 1U;
    phase = 0.0;
}

float Apu::NoiseChannel::sample(double cpu_frequency, double sample_rate) noexcept {
    if (!enabled || !length_counter.active()) {
        return 0.0F;
    }

    const u8 volume = envelope.output_volume();
    if (volume == 0U) {
        return 0.0F;
    }

    const double period_cycles =
        apu_tables::kNoisePeriodsCpuCycles[period_index & 0x0FU];

    const double frequency = cpu_frequency / period_cycles;

    phase += frequency / sample_rate;

    while (phase >= 1.0) {
        phase -= 1.0;

        const u16 tap = mode ? 6U : 1U;
        const u16 feedback = static_cast<u16>(
            (shift_register ^ (shift_register >> tap)) & 0x0001U
        );

        shift_register = static_cast<u16>((shift_register >> 1U) | (feedback << 14U));

        if (shift_register == 0U) {
            shift_register = 1U;
        }
    }

    const bool output_low = (shift_register & 0x0001U) == 0U;
    return output_low ? static_cast<float>(volume) : 0.0F;
}

u8 Apu::cpu_read(u16 addr) {
    if (addr == 0x4015U) {
        u8 status = 0;

        if (pulse1_.length_counter.active()) {
            status = static_cast<u8>(status | 0x01U);
        }

        if (pulse2_.length_counter.active()) {
            status = static_cast<u8>(status | 0x02U);
        }

        if (triangle_.length_counter.active()) {
            status = static_cast<u8>(status | 0x04U);
        }

        if (noise_.length_counter.active()) {
            status = static_cast<u8>(status | 0x08U);
        }

        if (frame_irq_pending_) {
            status = static_cast<u8>(status | 0x40U);
        }

        frame_irq_pending_ = false;
        return status;
    }

    return 0;
}

void Apu::cpu_write(u16 addr, u8 value) {
    switch (addr) {
    case 0x4000U:
        pulse1_.write_control(value);
        break;

    case 0x4001U:
        pulse1_.write_sweep(value);
        break;

    case 0x4002U:
        pulse1_.write_timer_low(value);
        break;

    case 0x4003U:
        pulse1_.write_timer_high(value);
        break;

    case 0x4004U:
        pulse2_.write_control(value);
        break;

    case 0x4005U:
        pulse2_.write_sweep(value);
        break;

    case 0x4006U:
        pulse2_.write_timer_low(value);
        break;

    case 0x4007U:
        pulse2_.write_timer_high(value);
        break;

    case 0x4008U:
        triangle_.write_linear_control(value);
        break;

    case 0x400AU:
        triangle_.write_timer_low(value);
        break;

    case 0x400BU:
        triangle_.write_timer_high(value);
        break;

    case 0x400CU:
        noise_.write_control(value);
        break;

    case 0x400EU:
        noise_.write_period(value);
        break;

    case 0x400FU:
        noise_.write_length(value);
        break;

    case 0x4015U:
        pulse1_.enabled = (value & 0x01U) != 0U;
        pulse2_.enabled = (value & 0x02U) != 0U;
        triangle_.enabled = (value & 0x04U) != 0U;
        noise_.enabled = (value & 0x08U) != 0U;

        pulse1_.length_counter.set_enabled(pulse1_.enabled);
        pulse2_.length_counter.set_enabled(pulse2_.enabled);
        triangle_.length_counter.set_enabled(triangle_.enabled);
        noise_.length_counter.set_enabled(noise_.enabled);

        if (!pulse1_.enabled) {
            pulse1_.phase = 0.0;
        }

        if (!pulse2_.enabled) {
            pulse2_.phase = 0.0;
        }

        if (!triangle_.enabled) {
            triangle_.phase = 0.0;
            triangle_.linear_counter = 0;
        }

        if (!noise_.enabled) {
            noise_.phase = 0.0;
            noise_.shift_register = 1U;
        }
        break;

    case 0x4017U:
        write_frame_counter(value);
        break;

    default:
        break;
    }
}

void Apu::write_frame_counter(u8 value) noexcept {
    frame_counter_five_step_ = (value & 0x80U) != 0U;
    frame_irq_inhibit_ = (value & 0x40U) != 0U;

    if (frame_irq_inhibit_) {
        frame_irq_pending_ = false;
    }

    frame_counter_cycle_ = 0;

    if (frame_counter_five_step_) {
        clock_quarter_frame();
        clock_half_frame();
    }
}

void Apu::tick(usize cpu_cycles) {
    tick_frame_counter(cpu_cycles);

    sample_accumulator_ += static_cast<double>(cpu_cycles);

    while (sample_accumulator_ >= kCpuCyclesPerSample) {
        sample_accumulator_ -= kCpuCyclesPerSample;
        sample_buffer_.push_back(mix_sample());

        if (sample_buffer_.size() > 48000U) {
            sample_buffer_.erase(sample_buffer_.begin(), sample_buffer_.begin() + 24000);
        }
    }
}

void Apu::tick_frame_counter(usize cpu_cycles) noexcept {
    for (usize i = 0; i < cpu_cycles; ++i) {
        ++frame_counter_cycle_;

        if (frame_counter_five_step_) {
            if (frame_counter_cycle_ == kFrameStep1 ||
                frame_counter_cycle_ == kFrameStep3) {
                clock_quarter_frame();
            } else if (frame_counter_cycle_ == kFrameStep2 ||
                       frame_counter_cycle_ == kFrameStep5) {
                clock_quarter_frame();
                clock_half_frame();
            }

            if (frame_counter_cycle_ >= kFrameStep5) {
                frame_counter_cycle_ = 0;
            }
        } else {
            if (frame_counter_cycle_ == kFrameStep1 ||
                frame_counter_cycle_ == kFrameStep3) {
                clock_quarter_frame();
            } else if (frame_counter_cycle_ == kFrameStep2) {
                clock_quarter_frame();
                clock_half_frame();
            } else if (frame_counter_cycle_ == kFrameStep4) {
                clock_quarter_frame();
                clock_half_frame();

                if (!frame_irq_inhibit_) {
                    frame_irq_pending_ = true;
                }

                frame_counter_cycle_ = 0;
            }
        }
    }
}

void Apu::clock_quarter_frame() noexcept {
    pulse1_.envelope.tick_quarter_frame();
    pulse2_.envelope.tick_quarter_frame();
    noise_.envelope.tick_quarter_frame();
    triangle_.tick_quarter_frame();
}

void Apu::clock_half_frame() noexcept {
    pulse1_.tick_half_frame();
    pulse2_.tick_half_frame();
    triangle_.length_counter.tick_half_frame(triangle_.control_flag);
    noise_.length_counter.tick_half_frame(noise_.envelope.loop_flag());
}

float Apu::mix_sample() noexcept {
    const float p1 = pulse1_.sample(kCpuFrequency, kSampleRate);
    const float p2 = pulse2_.sample(kCpuFrequency, kSampleRate);
    const float tri = triangle_.sample(kCpuFrequency, kSampleRate);
    const float noi = noise_.sample(kCpuFrequency, kSampleRate);

    const float pulse_sum = p1 + p2;
    const float pulse_out = pulse_sum > 0.0F
        ? 95.88F / ((8128.0F / pulse_sum) + 100.0F)
        : 0.0F;

    const float tnd_sum = (tri / 8227.0F) + (noi / 12241.0F);
    const float tnd_out = tnd_sum > 0.0F
        ? 159.79F / ((1.0F / tnd_sum) + 100.0F)
        : 0.0F;

    return apply_output_filters(pulse_out + tnd_out);
}

std::vector<float> Apu::take_samples() {
    std::vector<float> out;
    out.swap(sample_buffer_);
    return out;
}

} // namespace nes