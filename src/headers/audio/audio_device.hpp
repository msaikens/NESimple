#pragma once

#include <vector>

namespace nes {

class AudioDevice {
public:
    AudioDevice();
    ~AudioDevice();

    AudioDevice(const AudioDevice&) = delete;
    AudioDevice& operator=(const AudioDevice&) = delete;

    [[nodiscard]] bool available() const noexcept;

    [[nodiscard]] int queued_sample_count() const noexcept;
    [[nodiscard]] int pending_sample_count() const noexcept;
    [[nodiscard]] int target_queued_sample_count() const noexcept;
    [[nodiscard]] int max_queued_sample_count() const noexcept;

    void push_samples(const std::vector<float>& samples);
    void push_silence(int sample_count);

private:
    void pump_pending_samples();

    void* device_ {nullptr};
    void* stream_ {nullptr};
    bool available_ {false};

    std::vector<float> pending_samples_ {};

    static constexpr int kTargetQueuedSamples = 4096;
    static constexpr int kMaxQueuedSamples = 8192;
    static constexpr int kMaxPendingSamples = 48000;
};

} // namespace nes