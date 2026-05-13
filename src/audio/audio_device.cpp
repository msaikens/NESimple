#include "audio/audio_device.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#if defined(NESIMPLE_HAS_SDL)
#include <SDL3/SDL.h>
#endif

namespace nes {

AudioDevice::AudioDevice() {
#if defined(NESIMPLE_HAS_SDL)
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        return;
    }

    SDL_AudioSpec spec {};
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq = 48000;

    const SDL_AudioDeviceID device =
        SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);

    if (device == 0) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &spec);
    if (stream == nullptr) {
        SDL_CloseAudioDevice(device);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    if (!SDL_BindAudioStream(device, stream)) {
        SDL_DestroyAudioStream(stream);
        SDL_CloseAudioDevice(device);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    SDL_ResumeAudioDevice(device);

    device_ = reinterpret_cast<void*>(static_cast<std::uintptr_t>(device));
    stream_ = stream;
    available_ = true;

    pending_samples_.reserve(8192);

    push_silence(kTargetQueuedSamples);
#endif
}

AudioDevice::~AudioDevice() {
#if defined(NESIMPLE_HAS_SDL)
    if (stream_ != nullptr) {
        SDL_DestroyAudioStream(static_cast<SDL_AudioStream*>(stream_));
        stream_ = nullptr;
    }

    if (device_ != nullptr) {
        const auto device = static_cast<SDL_AudioDeviceID>(
            reinterpret_cast<std::uintptr_t>(device_)
        );

        SDL_CloseAudioDevice(device);
        device_ = nullptr;
    }

    if (available_) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
#endif
}

bool AudioDevice::available() const noexcept {
    return available_;
}

int AudioDevice::queued_sample_count() const noexcept {
#if defined(NESIMPLE_HAS_SDL)
    if (!available_ || stream_ == nullptr) {
        return 0;
    }

    const int queued_bytes =
        SDL_GetAudioStreamQueued(static_cast<SDL_AudioStream*>(stream_));

    if (queued_bytes <= 0) {
        return 0;
    }

    return queued_bytes / static_cast<int>(sizeof(float));
#else
    return 0;
#endif
}

int AudioDevice::pending_sample_count() const noexcept {
    return static_cast<int>(pending_samples_.size());
}

int AudioDevice::target_queued_sample_count() const noexcept {
    return kTargetQueuedSamples;
}

int AudioDevice::max_queued_sample_count() const noexcept {
    return kMaxQueuedSamples;
}

void AudioDevice::push_samples(const std::vector<float>& samples) {
#if defined(NESIMPLE_HAS_SDL)
    if (!available_ || stream_ == nullptr || samples.empty()) {
        return;
    }

    pending_samples_.insert(
        pending_samples_.end(),
        samples.begin(),
        samples.end()
    );

    // Emergency protection only. Normal pacing should never reach this.
    if (pending_samples_.size() > static_cast<std::size_t>(kMaxPendingSamples)) {
        const auto excess =
            pending_samples_.size() - static_cast<std::size_t>(kMaxPendingSamples);

        pending_samples_.erase(
            pending_samples_.begin(),
            pending_samples_.begin() + static_cast<std::ptrdiff_t>(excess)
        );
    }

    pump_pending_samples();
#else
    static_cast<void>(samples);
#endif
}

void AudioDevice::push_silence(int sample_count) {
#if defined(NESIMPLE_HAS_SDL)
    if (!available_ || stream_ == nullptr || sample_count <= 0) {
        return;
    }

    const std::vector<float> silence(static_cast<std::size_t>(sample_count), 0.0F);

    SDL_PutAudioStreamData(
        static_cast<SDL_AudioStream*>(stream_),
        silence.data(),
        static_cast<int>(silence.size() * sizeof(float))
    );
#else
    static_cast<void>(sample_count);
#endif
}

void AudioDevice::pump_pending_samples() {
#if defined(NESIMPLE_HAS_SDL)
    if (!available_ || stream_ == nullptr || pending_samples_.empty()) {
        return;
    }

    const int queued = queued_sample_count();

    if (queued >= kMaxQueuedSamples) {
        return;
    }

    const int room = kMaxQueuedSamples - queued;
    const int to_send = std::min(
        room,
        static_cast<int>(pending_samples_.size())
    );

    if (to_send <= 0) {
        return;
    }

    SDL_PutAudioStreamData(
        static_cast<SDL_AudioStream*>(stream_),
        pending_samples_.data(),
        to_send * static_cast<int>(sizeof(float))
    );

    pending_samples_.erase(
        pending_samples_.begin(),
        pending_samples_.begin() + to_send
    );
#endif
}

} // namespace nes