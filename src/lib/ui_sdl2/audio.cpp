/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "ui_sdl2/audio.hpp"

#include <algorithm>
#include <sstream>

#include "logger.hpp"
#include "ui_sdl2/sdl2.hpp"


namespace caio {
namespace ui {
namespace sdl2 {

AudioStream::AudioStream()
{
}

AudioStream::~AudioStream()
{
}

void AudioStream::reset(const ui::AudioConfig &aconf)
{
    if (_devid) {
        SDL_CloseAudioDevice(_devid);
        _devid = 0;
    }

    if (!aconf.enabled) {
        return;
    }

    SDL_AudioSpec obtained{};
    SDL_AudioSpec desired{};

    desired.freq     = aconf.srate;
    desired.format   = AUDIO_S16SYS;  /* Native endian signed 16 bits */
    desired.channels = aconf.channels;
    desired.samples  = aconf.samples;
    desired.userdata = static_cast<void *>(this);
    desired.callback = reinterpret_cast<SDL_AudioCallback>(AudioStream::stream_data);

    _devid = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (_devid == 0) {
        throw_sdl_uierror("Can't open audio device");
    }

    if (obtained.freq != desired.freq ||
        obtained.format != desired.format ||
        obtained.channels != desired.channels ||
        obtained.samples != desired.samples) {

        std::ostringstream os{};

        os << "Can't set audio parameters: Desired"
           << ": srate: "    << desired.freq
           << ", format: "   << desired.format
           << ", channels: " << desired.channels
           << ", samples: "  << desired.samples
           << ". Obtained"
           << ": srate: "    << obtained.freq
           << ", format: "   << obtained.format
           << ", channels: " << obtained.channels
           << ", samples: "  << obtained.samples;

        throw_sdl_uierror(os.str());
    }

    _playing_queue.clear();
    _free_queue.clear();

    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));

    _stop = false;
}

void AudioStream::stop()
{
    _stop = true;

    if (_devid) {
        /* Blocks until all audio related events are stopped */
        SDL_CloseAudioDevice(_devid);
        _devid = 0;
    }
}

void AudioStream::play()
{
    SDL_PauseAudioDevice(_devid, 0);
    _paused = false;
}

void AudioStream::pause()
{
    SDL_PauseAudioDevice(_devid, 1);
    _paused = true;
}

bool AudioStream::is_paused() const
{
    return _paused;
}

void AudioStream::volume(float vol)
{
    _volume = std::max(0.0f, std::min(1.0f, vol));
}

float AudioStream::volume() const
{
    return _volume;
}

AudioBuffer AudioStream::buffer()
{
    static auto dispatcher = [this](samples_i16 &&buf) {
        _playing_queue.push(std::move(buf));
    };

    if (_stop) {
        return {{}, {}};
    }

    if (_free_queue.size() == 0) {
//        log.warn("ui: audio: No free buffers available\n");
        return {{}, {}};
    }

    return {dispatcher, _free_queue.pop()};
}

void AudioStream::stream_data(AudioStream *self, uint8_t *stream, int len)
{
    if (self->_stop || self->_playing_queue.size() == 0) {
        std::fill_n(stream, len, 0);
        return;
    }

    const auto samples = self->_playing_queue.pop();
    int16_t *data = reinterpret_cast<int16_t *>(stream);
    size_t datasiz = std::min<size_t>(len >> 1, samples.size());;
    if (datasiz < samples.size()) {
        log.warn("ui: audio: Destination buffer size: %d, expected %d. Audio stream truncated\n",
            datasiz,
            samples.size());
    }

    auto it = samples.begin();
    while (datasiz--) {
        *data++ = *it * self->_volume;
        ++it;
    }

    self->_free_queue.push(std::move(samples));
}

}
}
}
