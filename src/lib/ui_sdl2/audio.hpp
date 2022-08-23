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
    constexpr static float DEFAULT_VOLUME = 0.5f;

    /**
     * Initialise an audio stream.
     * The reset() method must be called before using ths instance for the first time.
     * @see reset()
     */
    AudioStream();

    virtual ~AudioStream();

    /**
     * Reset this audio stream.
     * @param aconf Audio configuration.
     * @exception UIError
     */
    void reset(const ui::AudioConfig &aconf);

    /**
     * Stop the audio stream.
     * Block until all audio related functions are seased and close the audio device.
     */
    void stop();

    /**
     * Start playing.
     */
    void play();

    /**
     * Pause playing;
     */
    void pause();

    /**
     * @return true if this audio stream is paused; flase otherwise.
     */
    bool is_paused() const;

    /**
     * Set the output volume;
     * @param vol Volume (between 0 and 1);
     */
    void volume(float vol);

    /**
     * @return The current auto stream volume.
     */
    float volume() const;

    /**
     * Get a free audio buffer.
     * The audio buffer must be filled with audio samples and then dispatched (returned back).
     * When dispatched, the audio buffer automatically enqueues the* buffer into the playing
     * queue of this audio stream instance.
     * @return An AudioBuffer
     * @see AudioBuffer
     * @see stream_data()
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
    static void stream_data(AudioStream *self, uint8_t *stream, int len);

    std::atomic_bool         _stop{};
    bool                     _paused{true};
    SDL_AudioDeviceID        _devid{};
    LockedQueue<samples_i16> _free_queue{};
    LockedQueue<samples_i16> _playing_queue{};
    float                    _volume{DEFAULT_VOLUME};
};

}
}
}
