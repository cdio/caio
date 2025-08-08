/*
 * Copyright (C) 2020 Claudio Castiglia
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

void AudioStream::reset(const ui::AudioConfig& aconf)
{
    if (_devid) {
        ::SDL_CloseAudioDevice(_devid);
        _devid = 0;
    }

    if (!aconf.enabled) {
        return;
    }

    ::SDL_AudioSpec obtained{};
    ::SDL_AudioSpec desired{
        .freq     = static_cast<int>(aconf.srate),
        .format   = AUDIO_S16SYS,  /* Native endian signed 16 bits */
        .channels = static_cast<uint8_t>(aconf.channels),
        .samples  = static_cast<uint16_t>(aconf.samples),
        .callback = reinterpret_cast<::SDL_AudioCallback>(AudioStream::stream_data),
        .userdata = static_cast<void*>(this)
    };

    _devid = ::SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (_devid == 0) {
        throw UIError{"Can't open audio device: {}", sdl_error()};
    }

    if (obtained.freq != desired.freq ||
        obtained.format != desired.format ||
        obtained.channels != desired.channels ||
        obtained.samples != desired.samples) {
        throw UIError{"Can't set audio parameters: Desired: "
            "srate {}, format {}, channels {}, samples {}. Obtained: "
            "srate {}, format {}, channels {}, samples {}: {}",
            desired.freq, desired.format, desired.channels, desired.samples,
            obtained.freq, obtained.format, obtained.channels, obtained.samples,
            sdl_error()};
    }

    _playing_queue.clear();
    _free_queue.clear();

    std::for_each(_buffers, _buffers + std::size(_buffers), [this, &aconf](std::vector<int16_t>& buf) {
        buf.resize(aconf.samples);
        this->_free_queue.push({buf.data(), buf.size()});
    });

    _stop = false;
}

void AudioStream::stop()
{
    _stop = true;

    if (_devid) {
        /* Block until all audio related events are stopped */
        ::SDL_CloseAudioDevice(_devid);
        _devid = 0;
    }
}

void AudioStream::play()
{
    ::SDL_PauseAudioDevice(_devid, 0);
    _paused = false;
}

void AudioStream::pause()
{
    ::SDL_PauseAudioDevice(_devid, 1);
    _paused = true;
}

bool AudioStream::is_paused() const
{
    return _paused;
}

void AudioStream::volume(float vol)
{
    _volume = std::clamp(vol, 0.0f, 1.0f);
}

float AudioStream::volume() const
{
    return _volume;
}

AudioBuffer AudioStream::buffer()
{
    auto dispatcher = [this](samples_i16&& buf) {
        _playing_queue.push(std::move(buf));
    };

    if (!_stop && _free_queue.size() != 0) {
        if (auto buf = _free_queue.pop(); buf.has_value()) {
            return {dispatcher, std::move(buf.value())};
        }
    }

    return {{}, {}};
}

void AudioStream::stream_data(AudioStream* self, uint8_t* stream, int len)
{
    if (self->_stop || self->_playing_queue.size() == 0) {
        std::fill_n(stream, len, 0);
        return;
    }

    auto osamples = self->_playing_queue.pop();
    if (!osamples) {
        /*
         * No more buffers to play.
         */
        return;
    }

    auto samples = osamples.value();
    int16_t* data = reinterpret_cast<int16_t*>(stream);
    size_t datasiz = std::min<size_t>(len >> 1, samples.size());
    if (datasiz < samples.size()) {
        log.warn("ui: audio: Destination buffer size: {}, expected: {}. Audio stream truncated\n",
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
