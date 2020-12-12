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

#include <algorithm>

#include "icon.hpp"
#include "logger.hpp"


namespace cemu {
namespace ui {
namespace sfml {

std::stringstream sfml_err{};


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

    /* KEY_LT missing on SFML; see _unknown_key_pressed */
};


std::shared_ptr<UI> UISfml::create(const ui::Config &conf)
{
    auto ui = std::make_shared<UISfml>(conf);
    ui->icon(icon32());
    return ui;
}

Keyboard::Key UISfml::to_key(const sf::Keyboard::Key &code)
{
    auto it = sfml_to_key.find(code);
    return (it == sfml_to_key.end() ? Keyboard::KEY_NONE : it->second);
}

sf::Vector2u UISfml::window_size(bool panel_visible, const sf::Vector2u &screen_size)
{
    return {screen_size.x, screen_size.y + PanelSfml::size(panel_visible, screen_size.x).y};
}

UISfml::UISfml(const Config &conf)
    : UI{conf}
{
    sf::err().rdbuf(sfml_err.rdbuf());

    const auto &vconf = conf.video;
    const auto &aconf = conf.audio;

    _screen_size = {
        static_cast<unsigned>(vconf.width * vconf.scale),
        static_cast<unsigned>(vconf.height * vconf.scale)
    };

    _screen_ratio = static_cast<float>(vconf.width) / static_cast<float>(vconf.height);
    _scale = {vconf.scale, vconf.scale};

    _win_size = UISfml::window_size(vconf.panel, _screen_size);

    _window.create(sf::VideoMode{_win_size.x, _win_size.y}, vconf.title,
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

    _window.setVerticalSyncEnabled(false);
    _window.setFramerateLimit(vconf.fps);
    _window.setKeyRepeatEnabled(false);
    _window.clear(sf::Color::Black);

    _view.reset(sf::FloatRect{0.0f, 0.0f, static_cast<float>(_win_size.x), static_cast<float>(_win_size.y)});
    _window.setView(_view);

    _window.display();
    _window.setActive(false);

    _saved_win_pos = _window.getPosition();
    _saved_win_size = _win_size;

    _desktop_mode = sf::VideoMode::getDesktopMode();

    if (!_render_tex.create(_desktop_mode.width, _desktop_mode.height)) {
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

    _panel = std::make_shared<PanelSfml>(vconf.panel, _screen_size.x);
    if (!_panel) {
        throw UIError{"Can't instantiate SFML panel: " + Error::to_string()};
    }

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

sf::Sprite UISfml::render_screen()
{
    const auto &vconf = _conf.video;

    _render_tex.clear(sf::Color{CRT_COLOR});

    _screen_tex.update(reinterpret_cast<const uint8_t *>(_screen_raw.data()), vconf.width, vconf.height, 0, 0);
    sf::Sprite sprite{_screen_tex};
    sprite.setScale(_scale);
    sprite.setPosition(0, 0);

    _render_tex.draw(sprite);

    if (vconf.sleffect != ui::SLEffect::NONE) {
        sf::Sprite scanline_sprite{_scanline_tex};

        switch (vconf.sleffect) {
        case ui::SLEffect::HORIZONTAL:
            scanline_sprite.setScale(_screen_size.x, 1.0f);
            for (unsigned y = 0; y < _screen_size.y; y += _scale.y) {
                scanline_sprite.setPosition(0, y);
                _render_tex.draw(scanline_sprite);
                if (_scale.y == 1.0f) {
                    ++y;
                }
            }
            break;

        case ui::SLEffect::VERTICAL:
            scanline_sprite.setScale(1.0f, _screen_size.y);
            for (unsigned x = 0; x < _screen_size.x; x += _scale.x) {
                scanline_sprite.setPosition(x, 0);
                _render_tex.draw(scanline_sprite);
                if (_scale.x == 1.0f) {
                    ++x;
                }
            }
            break;

        default:;
        }
    }

    _render_tex.display();

    return sf::Sprite{_render_tex.getTexture()};
}

void UISfml::render_window()
{
    _window.clear();

    auto screen_sprite = render_screen();
    screen_sprite.setPosition(0, 0);
    _window.draw(screen_sprite);

    if (_panel->is_visible()) {
        auto panel_sprite = _panel->sprite();
        panel_sprite.setPosition(0, _screen_size.y);
        panel_sprite.setScale({1.0f, 1.0f});
        _window.draw(panel_sprite);
    }

    _window.display();
}

void UISfml::toggle_fullscreen()
{
    if (_is_fullscreen) {
        _win_size = _saved_win_size;

        _window.create(sf::VideoMode{_win_size.x, _win_size.y}, _conf.video.title,
            sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

        _window.setMouseCursorVisible(true);
        _window.setPosition(_saved_win_pos);

        if (_icon) {
            _window.setIcon(_icon.width, _icon.height, reinterpret_cast<const uint8_t *>(_icon.data.data()));
        }

        _is_fullscreen = false;

    } else {
        _saved_win_pos = _window.getPosition();
        _saved_win_size = _win_size;

        _window.create(_desktop_mode, _conf.video.title, sf::Style::Fullscreen);
        _window.setMouseCursorVisible(false);

        _is_fullscreen = true;
    }

    _window.setVerticalSyncEnabled(false);
    _window.setFramerateLimit(_conf.video.fps);
    _window.setKeyRepeatEnabled(false);
}

void UISfml::toggle_panel_visibility()
{
    if (_is_fullscreen) {
        _panel->visible(!_panel->is_visible());

        auto wsize = _window.getSize();
        resize_event(wsize.x, wsize.y);

    } else {
        auto wsize = _window.getSize();

        if (_panel->is_visible()) {
            wsize.y -= _panel->size().y;
            _panel->visible(false);
        } else {
            _panel->visible(true);
            wsize.y += _panel->size().y;
        }

        _window.setSize(wsize);
    }
}

void UISfml::resize_event(unsigned rwidth, unsigned rheight)
{
    const auto &vconf = _conf.video;

    auto min_size = UISfml::window_size(_panel->is_visible(), {vconf.width, vconf.height});
    auto width = std::max(min_size.x, rwidth);
    auto height = std::max(min_size.y, rheight);

    width = std::min(width, _desktop_mode.width);
    height = std::min(height, _desktop_mode.height);

    if (_panel->is_visible() && height > _panel->size().y) {
        height -= _panel->size().y;
    }

    unsigned sheight = width / _screen_ratio;
    if (sheight <= vconf.height) {
        _screen_size = {width, sheight};
    } else {
        _screen_size= {static_cast<unsigned>(height * _screen_ratio), height};
    }

    _scale.x = _scale.y = _screen_size.x / static_cast<float>(vconf.width);
    if (!vconf.smooth_resize) {
        _scale.x = _scale.y = std::ceil(_scale.x);
    }

    _win_size = UISfml::window_size(_panel->is_visible(), _screen_size);
    _panel->resize(_screen_size.x);

    _view.reset(sf::FloatRect{0.0f, 0.0f, static_cast<float>(rwidth), static_cast<float>(rheight)});

    float cx = (static_cast<float>(rwidth) - static_cast<float>(_screen_size.x)) / 2.0f;
    if (cx > 0.0f) {
        _view.move(-cx, 0);
    }

    _window.clear(sf::Color::Black);
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
                    /* Pause emulator */
                    hotkey(Keyboard::KEY_PAUSE);
                    break;

                case sf::Keyboard::V:
                    /* Toggle panel visibility */
                    toggle_panel_visibility();
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
            log.debug("Joystick " + std::to_string(jid) + " connected\n");
            joy->reset(jid);
            break;

        case sf::Event::JoystickDisconnected:
            log.debug("Joystick " + std::to_string(jid) + " disconnected\n");
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
            audio_stop();
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

void UISfml::icon(const Image &img)
{
    if (img) {
        _icon = img;
        _window.setIcon(img.width, img.height, reinterpret_cast<const uint8_t *>(img.data.data()));
    }
}

void UISfml::joystick(const std::initializer_list<std::shared_ptr<Joystick>> &il)
{
    UI::joystick(il);
    for (unsigned id = 0; id < _joys.size(); ++id) {
        if (sf::Joystick::isConnected(id)) {
            _joys[id]->reset(id);
        } else {
            _joys[id]->reset();
        }
    }
}

}
}
}
