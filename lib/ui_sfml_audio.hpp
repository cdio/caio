/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#pragma once

#include <atomic>

#include <SFML/Audio.hpp>

#include "dispatcher.hpp"
#include "locked_queue.hpp"
#include "signal.hpp"
#include "ui_config.hpp"


namespace cemu {
namespace ui {
namespace sfml {

/**
 * Audio Buffer.
 * An audio buffer is a self-dispatchable object.
 * @see DispatcherT
 */
using AudioBuffer = DispatcherT<samples_i16>;


class AudioStream : public sf::SoundStream {
public:
    AudioStream()
        : sf::SoundStream{} {
    }

    virtual ~AudioStream() {
    }

    /**
     * Reset this audio stream.
     * @param aconf Audio configuration.
     */
    void reset(const ui::AudioConfig &aconf);

    void stop() override {
        _stop = true;
    }

    /**
     * Get a free audio buffer.
     * The returned audio buffer must be filled with audio samples and then dispatched (returned back).
     * The audio buffer's dispatch method enqueues the buffer into this audio stream's playing queue.
     * @return A dispatchable audio buffer.
     * @see onGetData()
     */
    AudioBuffer buffer();

private:
    /**
     * Audio samples provider.
     * This method is called by the SFML audio stream thread to get audio samples to play.
     * Sample buffers are retrieved from the playing queue and moved into the free queue.
     * Given that the queues contain several sample buffers it gives the SFML thread enough
     * time to copy the playing buffers before they are overwritten with new audio data
     * (this approach is not nice neither the behaviour of SFML).
     */
    bool onGetData(sf::SoundStream::Chunk &chk) override;

    void onSeek(sf::Time offset) override {
    }

    LockedQueue<samples_i16> _free_queue{};
    LockedQueue<samples_i16> _playing_queue{};
    std::atomic_bool         _stop{};
};

}
}
}
