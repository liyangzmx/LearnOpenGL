#ifndef BREAKOUT_AUDIO_ENGINE_H
#define BREAKOUT_AUDIO_ENGINE_H

#include <memory>

// Background music and overlapping one-shot effects, backed by miniaudio.
class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();
    void play2D(const char* path, bool loop = false);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

#endif
