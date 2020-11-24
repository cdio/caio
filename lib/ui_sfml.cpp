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
#include "ui_sfml.hpp"

#include <array>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "locked_queue.hpp"
#include "logger.hpp"
#include "types.hpp"

#define UISFML_VERSION      "SFML-" CEMU_STR(SFML_VERSION_MAJOR) "." \
                                    CEMU_STR(SFML_VERSION_MINOR) "." \
                                    CEMU_STR(SFML_VERSION_PATCH)

namespace cemu {
namespace ui {
namespace sfml {

using namespace std::chrono_literals;


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
    void reset(const ui::AudioConfig &aconf) {
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

    void stop() override {
        _stop = true;
        sf::SoundStream::stop();
    }

    /**
     * Get a free audio buffer.
     * The returned audio buffer must be filled with audio samples and then dispatched (returned back).
     * The audio buffer's dispatch method enqueues the buffer into this audio stream's playing queue.
     * @return A dispatchable audio buffer.
     * @see onGetData()
     */
    AudioBuffer buffer() {
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

private:
    /**
     * Audio samples provider.
     * This method is called by the SFML audio stream thread to get audio samples to play.
     * Sample buffers are retrieved from the playing queue and moved into the free queue.
     * Given that the queues contain several sample buffers it gives the SFML thread enough
     * time to copy the playing buffers before they are overwritten with new audio data
     * (this approach is not nice neither the behaviour of SFML).
     */
    bool onGetData(sf::SoundStream::Chunk &chk) override {
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

    void onSeek(sf::Time offset) override {
    }

    LockedQueue<samples_i16> _free_queue{};
    LockedQueue<samples_i16> _playing_queue{};
    std::atomic_bool         _stop{};
};


class UISfml : public UI {
public:
    explicit UISfml(const Config &conf);

    virtual ~UISfml() {
    }

    /**
     * @see UI::audio_play()
     */
    void audio_play() override {
        if (audio_enabled()) {
            _audio_stream.play();
        }
    }

    /**
     * @see UI::audio_pause()
     */
    void audio_pause() override {
        if (audio_enabled()) {
            _audio_stream.pause();
        }
    }

    /**
     * @see UI::audio_stop()
     */
    void audio_stop() override {
        if (audio_enabled()) {
            _audio_stream.stop();
        }
    }

    /**
     * @see UI::audio_volume(float)
     */
    void audio_volume(float vol) override {
        if (audio_enabled()) {
            _audio_stream.setVolume((vol > 1.0f) ? 100.0f : vol * 100.0f);
        }
    }

    /**
     * @see UI::audio_volume()
     */
    float audio_volume() const override {
        return (audio_enabled() ? _audio_stream.getVolume() / 100.0f : 0.0f);
    }

    /**
     * @see UI::audio_buffer()
     */
    AudioBuffer audio_buffer() override {
        if (audio_enabled()) {
            return _audio_stream.buffer();
        }

        return {{}, {}};
    }

    /**
     * @see UI::render_line()
     */
    void render_line(unsigned line, const Scanline &sline) override;

    /**
     * @see UI::process_events()
     */
    bool process_events() override;

    /**
     * @see UI::title()
     */
    void title(const std::string &title) override {
        _window.setTitle(title);
    }

    /**
     * @see UI::icon()
     */
    void icon(const Image &img) override {
        if (img) {
            _icon = img;
            _window.setIcon(img.width, img.height, reinterpret_cast<const uint8_t *>(img.data.data()));
        }
    }

    /**
     * @see UI::to_string()
     */
    std::string to_string() const override {
        return UISFML_VERSION;
    }

private:
    /**
     * Render the main window.
     * @see render_screen()
     */
    void render_window();

    /**
     * Render the texture screen into the UI window.
     * @see _screen_tex
     * @see render_line()
     */
    void render_screen();

    /**
     * Toggle the fullscreen mode.
     */
    void toggle_fullscreen();

    /**
     * Process the window resize event.
     * Adapt the video renderer to the new size.
     * @param w New absolute width;
     * @param h New absolute height.
     */
    void resize_event(unsigned w, unsigned h);

    /**
     * Keyboard event handler.
     * @param event SFML keyboard event.
     */
    void kbd_event(const sf::Event &event);

    /**
     * Joystick event handler.
     * @param event SFML joystick event.
     */
    void joy_event(const sf::Event &event);

    /**
     * Convert a SFML key code to Keyboard::Key code.
     * @param key SFML Key code.
     * @return Keyboard::Key code.
     */
    static Keyboard::Key to_key(const sf::Keyboard::Key &key);

    /**
     * Main window width.
     */
    unsigned _W{};

    /**
     * Main window height.
     */
    unsigned _H{};

    /**
     * Saved main window width (set when moving to fullscreen mode).
     */
    unsigned _saved_W{};

    /**
     * Saved main window height (set when moving to fullscreen mode).
     */
    unsigned _saved_H{};

    /**
     * Saved main window position (set when moving to fullscreen mode).
     */
    sf::Vector2i _saved_pos{};

    /**
     * Screen texture width.
     */
    unsigned _width{};

    /**
     * Screen texture height.
     */
    unsigned _height{};

    /**
     * Screen aspect ratio.
     * This value is fixed and calculated from the configuration.
     */
    float _aratio{};

    /**
     * Scale factor X.
     * This value depends on the Main window width.
     */
    float _scale_x{};

    /**
     * Scale factor Y.
     * This value depends on the Main window height.
     */
    float _scale_y{};

    /**
     * Fullscreen mode.
     */
    bool _is_fullscreen{};

    /**
     * Main window.
     */
    sf::RenderWindow _window{};

    /**
     * View Port.
     * Support for resizing events.
     */
    sf::View _view{};

    /**
     * Screen rendering buffer.
     * This texture is rendered in the main window at frame rate speed.
     */
    sf::RenderTexture _render_tex{};

    /**
     * Screen pixel data.
     * @see _screen_tex
     */
    std::vector<Rgba> _screen_raw{};

    /**
     * Screen texture.
     * This texture is updated with new pixel data at frame rate speed.
     * @see _screen_raw
     */
    sf::Texture _screen_tex{};

    /**
     * Texture for the scanline effect.
     */
    sf::Texture _scanline_tex{};

    /**
     * Window icon.
     */
    Image _icon{};

    /**
     * Audio output stream.
     */
    AudioStream _audio_stream{};

    /**
     * Work-around for the keyboard handling deficiency of SFML.
     */
    bool _unknown_key_pressed{};
    Keyboard::Key _unknown_key{Keyboard::KEY_NONE};

    /**
     * Conversion map from SFML key code to Keyboard::Key code.
     */
    static std::map<sf::Keyboard::Key, Keyboard::Key> sfml_to_key;

    /**
     * SFML error stream.
     */
    static std::stringstream sfml_err;
};


std::stringstream UISfml::sfml_err{};

std::map<sf::Keyboard::Key, Keyboard::Key> UISfml::sfml_to_key{
    { sf::Keyboard::Key::A,         Keyboard::KEY_A             },
    { sf::Keyboard::Key::B,         Keyboard::KEY_B             },
    { sf::Keyboard::Key::C,         Keyboard::KEY_C             },
    { sf::Keyboard::Key::D,         Keyboard::KEY_D             },
    { sf::Keyboard::Key::E,         Keyboard::KEY_E             },
    { sf::Keyboard::Key::F,         Keyboard::KEY_F             },
    { sf::Keyboard::Key::G,         Keyboard::KEY_G             },
    { sf::Keyboard::Key::H,         Keyboard::KEY_H             },
    { sf::Keyboard::Key::I,         Keyboard::KEY_I             },
    { sf::Keyboard::Key::J,         Keyboard::KEY_J             },
    { sf::Keyboard::Key::K,         Keyboard::KEY_K             },
    { sf::Keyboard::Key::L,         Keyboard::KEY_L             },
    { sf::Keyboard::Key::M,         Keyboard::KEY_M             },
    { sf::Keyboard::Key::N,         Keyboard::KEY_N             },
    { sf::Keyboard::Key::O,         Keyboard::KEY_O             },
    { sf::Keyboard::Key::P,         Keyboard::KEY_P             },
    { sf::Keyboard::Key::Q,         Keyboard::KEY_Q             },
    { sf::Keyboard::Key::R,         Keyboard::KEY_R             },
    { sf::Keyboard::Key::S,         Keyboard::KEY_S             },
    { sf::Keyboard::Key::T,         Keyboard::KEY_T             },
    { sf::Keyboard::Key::U,         Keyboard::KEY_U             },
    { sf::Keyboard::Key::V,         Keyboard::KEY_V             },
    { sf::Keyboard::Key::W,         Keyboard::KEY_W             },
    { sf::Keyboard::Key::X,         Keyboard::KEY_X             },
    { sf::Keyboard::Key::Y,         Keyboard::KEY_Y             },
    { sf::Keyboard::Key::Z,         Keyboard::KEY_Z             },
    { sf::Keyboard::Key::Num0,      Keyboard::KEY_0             },
    { sf::Keyboard::Key::Num1,      Keyboard::KEY_1             },
    { sf::Keyboard::Key::Num2,      Keyboard::KEY_2             },
    { sf::Keyboard::Key::Num3,      Keyboard::KEY_3             },
    { sf::Keyboard::Key::Num4,      Keyboard::KEY_4             },
    { sf::Keyboard::Key::Num5,      Keyboard::KEY_5             },
    { sf::Keyboard::Key::Num6,      Keyboard::KEY_6             },
    { sf::Keyboard::Key::Num7,      Keyboard::KEY_7             },
    { sf::Keyboard::Key::Num8,      Keyboard::KEY_8             },
    { sf::Keyboard::Key::Num9,      Keyboard::KEY_9             },
    { sf::Keyboard::Key::LBracket,  Keyboard::KEY_OPEN_BRACKET  },
    { sf::Keyboard::Key::RBracket,  Keyboard::KEY_CLOSE_BRACKET },
    { sf::Keyboard::Key::SemiColon, Keyboard::KEY_SEMICOLON     },
    { sf::Keyboard::Key::Comma,     Keyboard::KEY_COMMA         },
    { sf::Keyboard::Key::Period,    Keyboard::KEY_DOT           },
    { sf::Keyboard::Key::Quote,     Keyboard::KEY_APOSTROPHE    },
    { sf::Keyboard::Key::Slash,     Keyboard::KEY_SLASH         },
    { sf::Keyboard::Key::BackSlash, Keyboard::KEY_BACKSLASH     },
    { sf::Keyboard::Key::Tilde,     Keyboard::KEY_GRAVE_ACCENT  },
    { sf::Keyboard::Key::Equal,     Keyboard::KEY_EQUAL         },
    { sf::Keyboard::Key::Hyphen,    Keyboard::KEY_MINUS         },
    { sf::Keyboard::Key::Space,     Keyboard::KEY_SPACE         },
    { sf::Keyboard::Key::Tab,       Keyboard::KEY_TAB           },
    { sf::Keyboard::Key::Enter,     Keyboard::KEY_ENTER         },
    { sf::Keyboard::Key::Escape,    Keyboard::KEY_ESC           },
    { sf::Keyboard::Key::LControl,  Keyboard::KEY_LEFT_CTRL     },
    { sf::Keyboard::Key::LShift,    Keyboard::KEY_LEFT_SHIFT    },
    { sf::Keyboard::Key::LAlt,      Keyboard::KEY_LEFT_ALT      },
    { sf::Keyboard::Key::RControl,  Keyboard::KEY_RIGHT_CTRL    },
    { sf::Keyboard::Key::RShift,    Keyboard::KEY_RIGHT_SHIFT   },
    { sf::Keyboard::Key::RAlt,      Keyboard::KEY_RIGHT_ALT     },
    { sf::Keyboard::Key::Menu,      Keyboard::KEY_FN            },
    { sf::Keyboard::Key::BackSpace, Keyboard::KEY_BACKSPACE     },
    { sf::Keyboard::Key::PageUp,    Keyboard::KEY_PAGE_UP       },
    { sf::Keyboard::Key::PageDown,  Keyboard::KEY_PAGE_DOWN     },
    { sf::Keyboard::Key::End,       Keyboard::KEY_END           },
    { sf::Keyboard::Key::Home,      Keyboard::KEY_HOME          },
    { sf::Keyboard::Key::Insert,    Keyboard::KEY_INSERT        },
    { sf::Keyboard::Key::Delete,    Keyboard::KEY_DELETE        },
    { sf::Keyboard::Key::Left,      Keyboard::KEY_CURSOR_LEFT   },
    { sf::Keyboard::Key::Right,     Keyboard::KEY_CURSOR_RIGHT  },
    { sf::Keyboard::Key::Up,        Keyboard::KEY_CURSOR_UP     },
    { sf::Keyboard::Key::Down,      Keyboard::KEY_CURSOR_DOWN   },
    { sf::Keyboard::Key::F1,        Keyboard::KEY_F1            },
    { sf::Keyboard::Key::F2,        Keyboard::KEY_F2            },
    { sf::Keyboard::Key::F3,        Keyboard::KEY_F3            },
    { sf::Keyboard::Key::F4,        Keyboard::KEY_F4            },
    { sf::Keyboard::Key::F5,        Keyboard::KEY_F5            },
    { sf::Keyboard::Key::F6,        Keyboard::KEY_F6            },
    { sf::Keyboard::Key::F7,        Keyboard::KEY_F7            },
    { sf::Keyboard::Key::F8,        Keyboard::KEY_F8            },
    { sf::Keyboard::Key::F9,        Keyboard::KEY_F9            },
    { sf::Keyboard::Key::F10,       Keyboard::KEY_F10           },
    { sf::Keyboard::Key::F11,       Keyboard::KEY_F11           },
    { sf::Keyboard::Key::F12,       Keyboard::KEY_F12           }

    /* KEY_LT missing on SFML */
};


Keyboard::Key UISfml::to_key(const sf::Keyboard::Key &code)
{
    auto it = sfml_to_key.find(code);
    return (it == sfml_to_key.end() ? Keyboard::KEY_NONE : it->second);
}

UISfml::UISfml(const Config &conf)
    : UI{conf}
{
    sf::err().rdbuf(sfml_err.rdbuf());

    const auto &vconf = conf.video;
    const auto &aconf = conf.audio;

    _W       = vconf.width * vconf.scale;
    _H       = vconf.height * vconf.scale;
    _width   = _W;
    _height  = _H;
    _aratio  = static_cast<float>(_W) / static_cast<float>(_H);
    _scale_x = _scale_y = vconf.scale;

    _window.create(sf::VideoMode{_W, _H}, vconf.title, sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    _window.setVerticalSyncEnabled(false);
    _window.setFramerateLimit(vconf.fps);
    _window.setKeyRepeatEnabled(false);
    _window.clear(sf::Color::Black);

    _view.reset(sf::FloatRect{0.0f, 0.0f, static_cast<float>(_W), static_cast<float>(_H)});
    _window.setView(_view);

    _window.display();
    _window.setActive(false);

    _saved_pos = _window.getPosition();
    _saved_W   = _W;
    _saved_H   = _H;

    auto desktop_size = sf::VideoMode::getDesktopMode();
    if (!_render_tex.create(desktop_size.width, desktop_size.height)) {
        throw UIError{"Can't create the render texture: SFML: " + sfml_err.str()};
    }

    _render_tex.clear(sf::Color{CRT_COLOR});
    _render_tex.display();
    _render_tex.setActive(false);

    _screen_raw = std::vector<Rgba>(vconf.width * vconf.height);

    if (!_screen_tex.create(vconf.width, vconf.height)) {
        throw UIError{"Can't create the screen texture: SFML: " + sfml_err.str()};
    }

    if (!_scanline_tex.create(1, 1)) {
        throw UIError{"Can't create the scanline texture: SFML: " + sfml_err.str()};
    }

    Rgba slcolor{SCANLINE_COLOR};
    _scanline_tex.update(reinterpret_cast<const uint8_t *>(&slcolor));

    _is_fullscreen = false;
    if (vconf.fullscreen) {
        toggle_fullscreen();
    }

    if (aconf.enabled) {
        _audio_stream.reset(aconf);
    }
}

void UISfml::render_line(unsigned line, const Scanline &sline)
{
    if (_stop) {
        log.debug("UISfml::render_line: System is stopped.\n");
        return;
    }

    if (line >= _conf.video.height || sline.size() != _conf.video.width) {
        log.warn("UISfml::render_line: Invalid raster line %d, size %d. Ignored\n", line, sline.size());
        return;
    }

    std::copy(sline.begin(), sline.end(), _screen_raw.begin() + line * _conf.video.width);
}

void UISfml::render_screen()
{
    const auto &vconf = _conf.video;

    _render_tex.clear(sf::Color{CRT_COLOR});

    _screen_tex.update(reinterpret_cast<const uint8_t *>(_screen_raw.data()), vconf.width, vconf.height, 0, 0);
    sf::Sprite sprite{_screen_tex};
    sprite.setScale(_scale_x, _scale_y);
    sprite.setPosition(0, 0);

    _render_tex.draw(sprite);

    if (vconf.sleffect != ui::SLEffect::NONE) {
        sf::Sprite sprite{_scanline_tex};

        switch (vconf.sleffect) {
        case ui::SLEffect::HORIZONTAL:
            sprite.setScale(_width, 1);
            for (unsigned y = 0; y < _height; y += _scale_y) {
                sprite.setPosition(0, y);
                _render_tex.draw(sprite);
                if (_scale_y == 1) {
                    ++y;
                }
            }
            break;

        case ui::SLEffect::VERTICAL:
            sprite.setScale(1, _height);
            for (unsigned x = 0; x < _width; x += _scale_x) {
                sprite.setPosition(x, 0);
                _render_tex.draw(sprite);
                if (_scale_x == 1) {
                    ++x;
                }
            }
            break;

        default:;
        }
    }

    _render_tex.display();
}

void UISfml::render_window()
{
    render_screen();
    sf::Sprite sprite{_render_tex.getTexture()};

    auto [wx, wy] = _window.getSize();
    auto cx = ((wx > _width) ? (wx - _width) / 2 : 0);
    auto cy = ((wy > _height) ? (wy - _height) / 2 : 0);
    sprite.setPosition(cx, cy);

    _window.clear();
    _window.draw(sprite);
    _window.display();
}

void UISfml::toggle_fullscreen()
{
    const auto &title = _conf.video.title;

    if (_is_fullscreen) {
        _is_fullscreen = false;

        _W = _saved_W;
        _H = _saved_H;

        _window.create(sf::VideoMode{_W, _H}, title, sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
        _window.setMouseCursorVisible(true);
        _window.setPosition(_saved_pos);
        if (_icon) {
            _window.setIcon(_icon.width, _icon.height, reinterpret_cast<const uint8_t *>(_icon.data.data()));
        }

        resize_event(_W, _H);

    } else {
        _is_fullscreen = true;

        _saved_pos = _window.getPosition();
        _saved_W   = _W;
        _saved_H   = _H;

        auto desktop_size = sf::VideoMode().getDesktopMode();
        _window.create(desktop_size, title, sf::Style::Fullscreen);
        _window.setMouseCursorVisible(false);

        resize_event(desktop_size.width, desktop_size.height);
    }

    _window.setVerticalSyncEnabled(false);
    _window.setFramerateLimit(_conf.video.fps);
    _window.setKeyRepeatEnabled(false);
}

void UISfml::resize_event(unsigned w, unsigned h)
{
    const auto &vconf = _conf.video;

    if (w < vconf.width) {
        w = vconf.width;
    }

    if (h < vconf.height) {
        h = vconf.height;
    }

    _W = w;
    _H = h;
    unsigned H = w / _aratio;
    float scale{};
    if (H <= vconf.height) {
        _width = w;
        _height = H;
        scale = static_cast<float>(w) / static_cast<float>(vconf.width);
    } else {
        _width = h * _aratio;
        _height = h;
        scale = static_cast<float>(h) / static_cast<float>(vconf.height);
    }

    _scale_x = _scale_y = (vconf.smooth_resize ? scale : static_cast<unsigned>(std::ceil(scale)));

    _window.clear(sf::Color::Black);
    _view.reset(sf::FloatRect{0.0f, 0.0f, static_cast<float>(_W), static_cast<float>(_H)});
    _window.setView(_view);
}

void UISfml::kbd_event(const sf::Event &event)
{
    switch (event.type) {
    case sf::Event::KeyPressed:
        switch (event.key.code) {
        case sf::Keyboard::Key::F11:
            toggle_fullscreen();
            break;

        case sf::Keyboard::Key::Pause:
            hotkey(Keyboard::KEY_PAUSE);
            break;

        default:
            if (event.key.alt) {
                switch (event.key.code) {
                case sf::Keyboard::J:
                    /* Joystick Swap */
                    hotkey(Keyboard::KEY_ALT_J);
                    break;

                case sf::Keyboard::M:
                    /* Enter Monitor */
                    hotkey(Keyboard::KEY_ALT_M);
                    break;

                case sf::Keyboard::P:
                    hotkey(Keyboard::KEY_PAUSE);
                    break;

                default:;
                }

            } else if (event.key.code != sf::Keyboard::Unknown) {
                keyboard()->key_pressed(to_key(event.key.code));

            } else {
                /*
                 * Work-around for the keyboard handling deficiency of SFML.
                 */
                _unknown_key_pressed = true;
            }
        }
        break;

    case sf::Event::KeyReleased:
        if (event.key.code != sf::Keyboard::Unknown) {
            keyboard()->key_released(to_key(event.key.code));
        } else if (_unknown_key_pressed) {
            /*
             * Work-around for the keyboard handling deficiency of SFML.
             */
            keyboard()->key_released(_unknown_key);
            _unknown_key_pressed = false;
        }
        break;

    case sf::Event::TextEntered:
        /*
         * Work-around for the keyboard handling deficiency of SFML.
         */
        if (_unknown_key_pressed) {
            switch (event.text.unicode) {
            case '<':
            case '>':
                _unknown_key = Keyboard::KEY_LT;
                keyboard()->key_pressed(Keyboard::KEY_LT);
                break;

            default:
                _unknown_key_pressed = false;
                break;
            }
        }
        break;

    default:;
    }
}

void UISfml::joy_event(const sf::Event &event)
{
    unsigned jid = event.joystickConnect.joystickId;

    auto joy = joystick(jid);
    if (joy) {
        uint8_t pos = 0;

        switch (event.type) {
        case sf::Event::JoystickConnected:
        case sf::Event::JoystickDisconnected:
            log.debug("Joystick " + std::to_string(jid) +
                ((event.type == sf::Event::JoystickConnected) ? " connected\n" : " disconnected\n"));
            sf::Joystick::update();
            joy->reset();
            break;

        case sf::Event::JoystickButtonPressed:
            pos = Joystick::JOY_FIRE;
            break;

        case sf::Event::JoystickButtonReleased:
        case sf::Event::JoystickMoved:
            break;

        default:
            return;
        }

        float uipos = sf::Joystick::getAxisPosition(jid, sf::Joystick::Axis::PovX);
        if (uipos == 0.0f) {
            uipos = sf::Joystick::getAxisPosition(jid, sf::Joystick::Axis::X);
            if (uipos > -38.0f && uipos < 38.0f) {
                uipos = sf::Joystick::getAxisPosition(jid, sf::Joystick::Axis::U);
            }
        }

        if (uipos < -38.0f) {
            pos |= Joystick::JOY_LEFT;
        } else if (uipos > 38.0f) {
            pos |= Joystick::JOY_RIGHT;
        }

        uipos = sf::Joystick::getAxisPosition(jid, sf::Joystick::Axis::PovY);
        if (uipos == 0.0f) {
            uipos = sf::Joystick::getAxisPosition(jid, sf::Joystick::Axis::Y);
            if (uipos > -38.0f && uipos < 38.0f) {
                uipos = sf::Joystick::getAxisPosition(jid, sf::Joystick::Axis::V);
            }
        }

        if (uipos < -38.0f) {
            pos |= Joystick::JOY_UP;
        } else if (uipos > 38.0f) {
            pos |= Joystick::JOY_DOWN;
        }

        joy->position(pos);

    } else {
        log.warn("Jostick " + std::to_string(jid) + " is not supported\n");
    }
}

bool UISfml::process_events()
{
    _window.setActive(true);     //XXX

    sf::Event event{};
    while (_window.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            audio_stop();       /* Avoid SFML segfault? */
            _window.close();
            return false;

        case sf::Event::Resized:
            resize_event(event.size.width, event.size.height);
            break;

        case sf::Event::KeyPressed:
        case sf::Event::KeyReleased:
        case sf::Event::TextEntered:
            kbd_event(event);
            break;

        case sf::Event::JoystickConnected:
        case sf::Event::JoystickDisconnected:
        case sf::Event::JoystickButtonPressed:
        case sf::Event::JoystickButtonReleased:
        case sf::Event::JoystickMoved:
            joy_event(event);
            break;

        default:;
        }
    }

    if (_window.isOpen()) {
       render_window();
    }

    return true;
}


std::shared_ptr<UI> create(const ui::Config &conf, const Image &icon)
{
    auto ui = std::make_shared<UISfml>(conf);
    ui->icon(icon);
    return ui;
}

}
}
}
