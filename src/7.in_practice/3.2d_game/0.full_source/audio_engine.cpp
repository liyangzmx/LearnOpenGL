#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "audio_engine.h"
#include <iostream>

struct AudioEngine::Impl
{
    ma_engine engine{};
    ma_sound music{};
    bool ready = false;
    bool musicReady = false;
};

AudioEngine::AudioEngine() : impl(new Impl)
{
    impl->ready = ma_engine_init(nullptr, &impl->engine) == MA_SUCCESS;
    if (!impl->ready)
        std::cerr << "Audio device unavailable; continuing without sound\n";
}

AudioEngine::~AudioEngine()
{
    if (impl->musicReady)
        ma_sound_uninit(&impl->music);
    if (impl->ready)
        ma_engine_uninit(&impl->engine);
}

void AudioEngine::play2D(const char* path, bool loop)
{
    if (!impl->ready)
        return;
    if (!loop)
    {
        if (ma_engine_play_sound(&impl->engine, path, nullptr) != MA_SUCCESS)
            std::cerr << "Failed to play sound: " << path << '\n';
        return;
    }
    if (impl->musicReady)
        ma_sound_uninit(&impl->music);
    impl->musicReady = ma_sound_init_from_file(&impl->engine, path, MA_SOUND_FLAG_STREAM, nullptr, nullptr, &impl->music) == MA_SUCCESS;
    if (!impl->musicReady)
    {
        std::cerr << "Failed to load music: " << path << '\n';
        return;
    }
    ma_sound_set_looping(&impl->music, MA_TRUE);
    if (ma_sound_start(&impl->music) != MA_SUCCESS)
        std::cerr << "Failed to start music: " << path << '\n';
}
