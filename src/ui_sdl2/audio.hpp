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

#include "dispatcher.hpp"
#include "locked_queue.hpp"     //TODO XXX use lock-free queues
#include "signal.hpp"
#include "ui_config.hpp"

#include <SDL_audio.h>

#include <atomic>

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Audio Buffer.
 * The Audio Buffer is a self-dispatchable object.
 * @see DispatcherT
 */
using AudioBuffer = DispatcherT<samples_i16>;

/**
 * Audio Stream driver.
 */
class AudioStream {
public:
    constexpr static const float DEFAULT_VOLUME = 0.5f;
    constexpr static const size_t AUDIO_BUFFERS = 4;

    /**
     * Initialise this audio stream driver.
     * The reset() method must be called before using this instance for the first time.
     * @see reset()
     */
    AudioStream() = default;

    virtual ~AudioStream() = default;

    /**
     * Reset this audio stream.
     * @param aconf Audio configuration.
     * @exception UIError
     * @see ui::AudioConfig
     */
    void reset(const ui::AudioConfig& aconf);

    /**
     * Stop the audio stream.
     * Block until all audio related events are seased and close the audio device.
     */
    void stop();

    /**
     * Start the audio stream.
     */
    void play();

    /**
     * Pause the audio stream.
     */
    void pause();

    /**
     * Get the status of the audio stream.
     * @return true if the audio stream is paused; false otherwise.
     */
    bool is_paused() const;

    /**
     * Set the output volume.
     * @param vol Volume (between 0 and 1).
     */
    void volume(float vol);

    /**
     * Get the output volume.
     * @return The current volume.
     */
    float volume() const;

    /**
     * Get a free audio buffer.
     * A free audio buffer is retrieved from the free queue; the caller
     * must fill the received buffer with PCM audio samples and dispatch it.
     * The act of dispatching (or destroying) the audio buffer automatically
     * enqueues it inside the playing queue of this audio stream driver.
     * @return An empty audio buffer.
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
     * @param self   A pointer to this instance;
     * @param stream Destination buffer to fill with audio samples;
     * @param len    Size of the destination buffer.
     */
    static void stream_data(AudioStream* self, uint8_t* stream, int len);

    std::atomic_bool         _stop{};
    bool                     _paused{true};
    ::SDL_AudioDeviceID      _devid{};
    LockedQueue<samples_i16> _free_queue{};
    LockedQueue<samples_i16> _playing_queue{};
    std::vector<int16_t>     _buffers[AUDIO_BUFFERS] = {};
    float                    _volume{DEFAULT_VOLUME};
};

}
}
}
