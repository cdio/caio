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
#include "ui_sdl2/sdl2.hpp"

#include <map>


namespace caio {
namespace ui {
namespace sdl2 {

static std::map<SDL_Scancode, Keyboard::Key> sdl_to_key{
    { SDL_SCANCODE_A,               Keyboard::KEY_A                 },
    { SDL_SCANCODE_B,               Keyboard::KEY_B                 },
    { SDL_SCANCODE_C,               Keyboard::KEY_C                 },
    { SDL_SCANCODE_D,               Keyboard::KEY_D                 },
    { SDL_SCANCODE_E,               Keyboard::KEY_E                 },
    { SDL_SCANCODE_F,               Keyboard::KEY_F                 },
    { SDL_SCANCODE_G,               Keyboard::KEY_G                 },
    { SDL_SCANCODE_H,               Keyboard::KEY_H                 },
    { SDL_SCANCODE_I,               Keyboard::KEY_I                 },
    { SDL_SCANCODE_J,               Keyboard::KEY_J                 },
    { SDL_SCANCODE_K,               Keyboard::KEY_K                 },
    { SDL_SCANCODE_L,               Keyboard::KEY_L                 },
    { SDL_SCANCODE_M,               Keyboard::KEY_M                 },
    { SDL_SCANCODE_N,               Keyboard::KEY_N                 },
    { SDL_SCANCODE_O,               Keyboard::KEY_O                 },
    { SDL_SCANCODE_P,               Keyboard::KEY_P                 },
    { SDL_SCANCODE_Q,               Keyboard::KEY_Q                 },
    { SDL_SCANCODE_R,               Keyboard::KEY_R                 },
    { SDL_SCANCODE_S,               Keyboard::KEY_S                 },
    { SDL_SCANCODE_T,               Keyboard::KEY_T                 },
    { SDL_SCANCODE_U,               Keyboard::KEY_U                 },
    { SDL_SCANCODE_V,               Keyboard::KEY_V                 },
    { SDL_SCANCODE_W,               Keyboard::KEY_W                 },
    { SDL_SCANCODE_X,               Keyboard::KEY_X                 },
    { SDL_SCANCODE_Y,               Keyboard::KEY_Y                 },
    { SDL_SCANCODE_Z,               Keyboard::KEY_Z                 },

    { SDL_SCANCODE_1,               Keyboard::KEY_1                 },
    { SDL_SCANCODE_2,               Keyboard::KEY_2                 },
    { SDL_SCANCODE_3,               Keyboard::KEY_3                 },
    { SDL_SCANCODE_4,               Keyboard::KEY_4                 },
    { SDL_SCANCODE_5,               Keyboard::KEY_5                 },
    { SDL_SCANCODE_6,               Keyboard::KEY_6                 },
    { SDL_SCANCODE_7,               Keyboard::KEY_7                 },
    { SDL_SCANCODE_8,               Keyboard::KEY_8                 },
    { SDL_SCANCODE_9,               Keyboard::KEY_9                 },
    { SDL_SCANCODE_0,               Keyboard::KEY_0                 },

    { SDL_SCANCODE_RETURN,          Keyboard::KEY_ENTER             },
    { SDL_SCANCODE_ESCAPE,          Keyboard::KEY_ESC               },
    { SDL_SCANCODE_BACKSPACE,       Keyboard::KEY_BACKSPACE         },
    { SDL_SCANCODE_TAB,             Keyboard::KEY_TAB               },
    { SDL_SCANCODE_SPACE,           Keyboard::KEY_SPACE             },

    { SDL_SCANCODE_MINUS,           Keyboard::KEY_MINUS             },
    { SDL_SCANCODE_EQUALS,          Keyboard::KEY_EQUAL             },
    { SDL_SCANCODE_LEFTBRACKET,     Keyboard::KEY_OPEN_BRACKET      },
    { SDL_SCANCODE_RIGHTBRACKET,    Keyboard::KEY_CLOSE_BRACKET     },
    { SDL_SCANCODE_BACKSLASH,       Keyboard::KEY_BACKSLASH         },

    { SDL_SCANCODE_SEMICOLON,       Keyboard::KEY_SEMICOLON         },
    { SDL_SCANCODE_APOSTROPHE,      Keyboard::KEY_APOSTROPHE        },
    { SDL_SCANCODE_GRAVE,           Keyboard::KEY_GRAVE_ACCENT      },
    { SDL_SCANCODE_COMMA,           Keyboard::KEY_COMMA             },
    { SDL_SCANCODE_PERIOD,          Keyboard::KEY_DOT               },
    { SDL_SCANCODE_SLASH,           Keyboard::KEY_SLASH             },

    { SDL_SCANCODE_F1,              Keyboard::KEY_F1                },
    { SDL_SCANCODE_F2,              Keyboard::KEY_F2                },
    { SDL_SCANCODE_F3,              Keyboard::KEY_F3                },
    { SDL_SCANCODE_F4,              Keyboard::KEY_F4                },
    { SDL_SCANCODE_F5,              Keyboard::KEY_F5                },
    { SDL_SCANCODE_F6,              Keyboard::KEY_F6                },
    { SDL_SCANCODE_F7,              Keyboard::KEY_F7                },
    { SDL_SCANCODE_F8,              Keyboard::KEY_F8                },
    { SDL_SCANCODE_F9,              Keyboard::KEY_F9                },
    { SDL_SCANCODE_F10,             Keyboard::KEY_F10               },
    { SDL_SCANCODE_F11,             Keyboard::KEY_F11               },
    { SDL_SCANCODE_F12,             Keyboard::KEY_F12               },

    { SDL_SCANCODE_PAUSE,           Keyboard::KEY_PAUSE             },
    { SDL_SCANCODE_INSERT,          Keyboard::KEY_INSERT            },

    { SDL_SCANCODE_HOME,            Keyboard::KEY_HOME              },
    { SDL_SCANCODE_PAGEUP,          Keyboard::KEY_PAGE_UP           },
    { SDL_SCANCODE_DELETE,          Keyboard::KEY_DELETE            },
    { SDL_SCANCODE_END,             Keyboard::KEY_END               },
    { SDL_SCANCODE_PAGEDOWN,        Keyboard::KEY_PAGE_DOWN         },
    { SDL_SCANCODE_RIGHT,           Keyboard::KEY_CURSOR_RIGHT      },
    { SDL_SCANCODE_LEFT,            Keyboard::KEY_CURSOR_LEFT       },
    { SDL_SCANCODE_DOWN,            Keyboard::KEY_CURSOR_DOWN       },
    { SDL_SCANCODE_UP,              Keyboard::KEY_CURSOR_UP         },

    { SDL_SCANCODE_KP_DIVIDE,       Keyboard::KEY_NUMPAD_SLASH      },
    { SDL_SCANCODE_KP_MULTIPLY,     Keyboard::KEY_NUMPAD_ASTERISK   },
    { SDL_SCANCODE_KP_MINUS,        Keyboard::KEY_NUMPAD_MINUS      },
    { SDL_SCANCODE_KP_PLUS,         Keyboard::KEY_NUMPAD_PLUS       },
    { SDL_SCANCODE_KP_ENTER,        Keyboard::KEY_NUMPAD_ENTER      },
    { SDL_SCANCODE_KP_1,            Keyboard::KEY_NUMPAD_1          },
    { SDL_SCANCODE_KP_2,            Keyboard::KEY_NUMPAD_2          },
    { SDL_SCANCODE_KP_3,            Keyboard::KEY_NUMPAD_3          },
    { SDL_SCANCODE_KP_4,            Keyboard::KEY_NUMPAD_4          },
    { SDL_SCANCODE_KP_5,            Keyboard::KEY_NUMPAD_5          },
    { SDL_SCANCODE_KP_6,            Keyboard::KEY_NUMPAD_6          },
    { SDL_SCANCODE_KP_7,            Keyboard::KEY_NUMPAD_7          },
    { SDL_SCANCODE_KP_8,            Keyboard::KEY_NUMPAD_8          },
    { SDL_SCANCODE_KP_9,            Keyboard::KEY_NUMPAD_9          },
    { SDL_SCANCODE_KP_0,            Keyboard::KEY_NUMPAD_0          },
    { SDL_SCANCODE_KP_PERIOD,       Keyboard::KEY_NUMPAD_DOT        },

    { SDL_SCANCODE_NONUSBACKSLASH,  Keyboard::KEY_LT                },

    { SDL_SCANCODE_LCTRL,           Keyboard::KEY_LEFT_CTRL         },
    { SDL_SCANCODE_LSHIFT,          Keyboard::KEY_LEFT_SHIFT        },
    { SDL_SCANCODE_LALT,            Keyboard::KEY_LEFT_ALT          },

    { SDL_SCANCODE_RCTRL,           Keyboard::KEY_RIGHT_CTRL        },
    { SDL_SCANCODE_RSHIFT,          Keyboard::KEY_RIGHT_SHIFT       },
    { SDL_SCANCODE_RALT,            Keyboard::KEY_RIGHT_ALT         }

#if 0
    SDL_SCANCODE_BACKSLASH = 49,    // Backslash key on ISO keyboards and at the right on ANSI keyboards.
    SDL_SCANCODE_NONUSHASH = 50,    // ISO USB keyboards actually use this code instead of 49 for the same key
                                    // OSes I've seen treat the two codes identically.
    SDL_SCANCODE_CAPSLOCK = 57,
    SDL_SCANCODE_INSERT = 73,       // Insert on PC, help on some Mac keyboards (but does send code 73, not 117)
    SDL_SCANCODE_NUMLOCKCLEAR = 83, // Num lock on PC, clear on Mac keyboards

    SDL_SCANCODE_LGUI = 227,        // Windows, command (apple), meta
    SDL_SCANCODE_RGUI = 231,        // Windows, command (apple), meta
#endif
};

Keyboard::Key to_key(SDL_Scancode code)
{
    auto it = sdl_to_key.find(code);
    return (it == sdl_to_key.end() ? Keyboard::KEY_NONE : it->second);
}

std::string sdl_error()
{
    return SDL_GetError();
}

void throw_sdl_uierror(const std::string& reason)
{
    throw UIError{reason + ": " + sdl_error()};
}

bool in_rect(int x, int y, const SDL_Rect& rect)
{
    return ((x >= rect.x && x < rect.x + rect.w) && (y >= rect.y && y < rect.y + rect.h));
}

}
}
}
