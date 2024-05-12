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
#pragma once

#include <atomic>

#include <SDL_audio.h>

#include "dispatcher.hpp"
#include "locked_queue.hpp"     //TODO XXX use lock-free queues
#include "signal.hpp"
#include "ui_config.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Audio Buffer.
 * An audio buffer is a self-dispatchable object.
 * @see DispatcherT
 */
using AudioBuffer = DispatcherT<samples_i16>;

/**
 * Audio driver.
 */
class AudioStream {
public:
    constexpr static const float DEFAULT_VOLUME = 0.5f;
    constexpr static const size_t AUDIO_BUFFERS = 4;

    /**
     * Initialise an audio stream.
     * The reset() method must be called before using ths instance for the first time.
     * @see reset()
     */
    AudioStream() {
    }

    virtual ~AudioStream() {
    }

    /**
     * Reset this audio stream.
     * @param aconf Audio configuration.
     * @exception UIError
     */
    void reset(const ui::AudioConfig& aconf);

    /**
     * Stop the audio stream.
     * Block until all audio related functions are seased and close the audio device.
     */
    void stop();

    /**
     * Start audio streaming.
     */
    void play();

    /**
     * Pause audio streaming.
     */
    void pause();

    /**
     * Get the running status of this audio stream.
     * @return true if this audio stream is paused; false otherwise.
     */
    bool is_paused() const;

    /**
     * Set the output volume.
     * @param vol Volume (between 0 and 1).
     */
    void volume(float vol);

    /**
     * Get the volume of this audio stream.
     * @return The current volume.
     */
    float volume() const;

    /**
     * Get a free audio buffer.
     * A free audio buffer is retrieved from the free queue and returned back to the caller.
     * The caller must fill the buffer with audio samples and then dispatch or destroy it.
     * The act of dispatching (or destroying) the received buffer makes it to be
     * automatically enqueued inside the playing queue of this audio stream instance.
     * @return An AudioBuffer.
     * @see _free_queue
     * @see _playing_queue
     * @see stream_data()
     * @see AudioBuffer
     */
    AudioBuffer buffer();

private:
    /**
     * Audio samples provider.
     * This method is called by the SDL audio subsystem to get audio samples to play.
     * @param self   A pointer to this class;
     * @param stream Destination buffer to fill with audio samples;
     * @param len    Size of the destination buffer.
     */
    static void stream_data(AudioStream* self, uint8_t* stream, int len);

    std::atomic_bool         _stop{};
    bool                     _paused{true};
    SDL_AudioDeviceID        _devid{};
    LockedQueue<samples_i16> _free_queue{};
    LockedQueue<samples_i16> _playing_queue{};
    float                    _volume{DEFAULT_VOLUME};

    std::vector<int16_t> _buffers[AUDIO_BUFFERS] = {};
};

}
}
}
