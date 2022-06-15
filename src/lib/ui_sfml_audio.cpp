/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
#include "ui_sfml_audio.hpp"

#include <thread>

#include "logger.hpp"


namespace caio {
namespace ui {
namespace sfml {

using namespace std::chrono_literals;


void AudioStream::reset(const ui::AudioConfig &aconf)
{
    sf::SoundStream::stop();
    _playing_queue.clear();
    _free_queue.clear();
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _free_queue.push(samples_i16(aconf.samples));
    _stop = false;
    sf::SoundStream::initialize(aconf.channels, aconf.srate);
}

AudioBuffer AudioStream::buffer()
{
    static auto dispatcher = [this](samples_i16 &&buf) {
        _playing_queue.push(std::move(buf));
        if (sf::SoundStream::getStatus() == sf::SoundSource::Status::Stopped) {
            sf::SoundStream::play();
        }
    };

    while (_free_queue.size() == 0) {
        if (_stop) {
            return {{}, {}};
        }

        if (sf::SoundStream::getStatus() == sf::SoundSource::Status::Stopped && !_stop) {
            /*
             * For some reason, sometimes SFML stops the audio stream.
             * We just ignore SFML and start the stream again.
             */
            log.debug("SFML stopped the audio stream. Restarting...\n");
            sf::SoundStream::play();
        }

        std::this_thread::sleep_for(10ms);
    }

    return AudioBuffer{dispatcher, _free_queue.pop()};
}

bool AudioStream::onGetData(sf::SoundStream::Chunk &chk)
{
    while (_playing_queue.size() == 0) {
        if (sf::SoundStream::getStatus() == sf::SoundSource::Status::Stopped) {
            return false;
        }

        std::this_thread::sleep_for(10ms);
    }

    _free_queue.push(_playing_queue.pop());
    const auto &samples = _free_queue.back();

    chk.sampleCount = samples.size();
    chk.samples = samples.data();

    return true;
}

}
}
}
