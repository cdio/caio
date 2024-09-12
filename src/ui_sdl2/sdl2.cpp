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

static std::map<::SDL_Scancode, keyboard::Key> sdl_to_key{
    { SDL_SCANCODE_A,               keyboard::KEY_A                 },
    { SDL_SCANCODE_B,               keyboard::KEY_B                 },
    { SDL_SCANCODE_C,               keyboard::KEY_C                 },
    { SDL_SCANCODE_D,               keyboard::KEY_D                 },
    { SDL_SCANCODE_E,               keyboard::KEY_E                 },
    { SDL_SCANCODE_F,               keyboard::KEY_F                 },
    { SDL_SCANCODE_G,               keyboard::KEY_G                 },
    { SDL_SCANCODE_H,               keyboard::KEY_H                 },
    { SDL_SCANCODE_I,               keyboard::KEY_I                 },
    { SDL_SCANCODE_J,               keyboard::KEY_J                 },
    { SDL_SCANCODE_K,               keyboard::KEY_K                 },
    { SDL_SCANCODE_L,               keyboard::KEY_L                 },
    { SDL_SCANCODE_M,               keyboard::KEY_M                 },
    { SDL_SCANCODE_N,               keyboard::KEY_N                 },
    { SDL_SCANCODE_O,               keyboard::KEY_O                 },
    { SDL_SCANCODE_P,               keyboard::KEY_P                 },
    { SDL_SCANCODE_Q,               keyboard::KEY_Q                 },
    { SDL_SCANCODE_R,               keyboard::KEY_R                 },
    { SDL_SCANCODE_S,               keyboard::KEY_S                 },
    { SDL_SCANCODE_T,               keyboard::KEY_T                 },
    { SDL_SCANCODE_U,               keyboard::KEY_U                 },
    { SDL_SCANCODE_V,               keyboard::KEY_V                 },
    { SDL_SCANCODE_W,               keyboard::KEY_W                 },
    { SDL_SCANCODE_X,               keyboard::KEY_X                 },
    { SDL_SCANCODE_Y,               keyboard::KEY_Y                 },
    { SDL_SCANCODE_Z,               keyboard::KEY_Z                 },

    { SDL_SCANCODE_1,               keyboard::KEY_1                 },
    { SDL_SCANCODE_2,               keyboard::KEY_2                 },
    { SDL_SCANCODE_3,               keyboard::KEY_3                 },
    { SDL_SCANCODE_4,               keyboard::KEY_4                 },
    { SDL_SCANCODE_5,               keyboard::KEY_5                 },
    { SDL_SCANCODE_6,               keyboard::KEY_6                 },
    { SDL_SCANCODE_7,               keyboard::KEY_7                 },
    { SDL_SCANCODE_8,               keyboard::KEY_8                 },
    { SDL_SCANCODE_9,               keyboard::KEY_9                 },
    { SDL_SCANCODE_0,               keyboard::KEY_0                 },

    { SDL_SCANCODE_RETURN,          keyboard::KEY_ENTER             },
    { SDL_SCANCODE_ESCAPE,          keyboard::KEY_ESC               },
    { SDL_SCANCODE_BACKSPACE,       keyboard::KEY_BACKSPACE         },
    { SDL_SCANCODE_TAB,             keyboard::KEY_TAB               },
    { SDL_SCANCODE_SPACE,           keyboard::KEY_SPACE             },

    { SDL_SCANCODE_MINUS,           keyboard::KEY_MINUS             },
    { SDL_SCANCODE_EQUALS,          keyboard::KEY_EQUAL             },
    { SDL_SCANCODE_LEFTBRACKET,     keyboard::KEY_OPEN_BRACKET      },
    { SDL_SCANCODE_RIGHTBRACKET,    keyboard::KEY_CLOSE_BRACKET     },
    { SDL_SCANCODE_BACKSLASH,       keyboard::KEY_BACKSLASH         },

    { SDL_SCANCODE_SEMICOLON,       keyboard::KEY_SEMICOLON         },
    { SDL_SCANCODE_APOSTROPHE,      keyboard::KEY_APOSTROPHE        },
    { SDL_SCANCODE_GRAVE,           keyboard::KEY_GRAVE_ACCENT      },
    { SDL_SCANCODE_COMMA,           keyboard::KEY_COMMA             },
    { SDL_SCANCODE_PERIOD,          keyboard::KEY_DOT               },
    { SDL_SCANCODE_SLASH,           keyboard::KEY_SLASH             },

    { SDL_SCANCODE_F1,              keyboard::KEY_F1                },
    { SDL_SCANCODE_F2,              keyboard::KEY_F2                },
    { SDL_SCANCODE_F3,              keyboard::KEY_F3                },
    { SDL_SCANCODE_F4,              keyboard::KEY_F4                },
    { SDL_SCANCODE_F5,              keyboard::KEY_F5                },
    { SDL_SCANCODE_F6,              keyboard::KEY_F6                },
    { SDL_SCANCODE_F7,              keyboard::KEY_F7                },
    { SDL_SCANCODE_F8,              keyboard::KEY_F8                },
    { SDL_SCANCODE_F9,              keyboard::KEY_F9                },
    { SDL_SCANCODE_F10,             keyboard::KEY_F10               },
    { SDL_SCANCODE_F11,             keyboard::KEY_F11               },
    { SDL_SCANCODE_F12,             keyboard::KEY_F12               },

    { SDL_SCANCODE_PAUSE,           keyboard::KEY_PAUSE             },
    { SDL_SCANCODE_INSERT,          keyboard::KEY_INSERT            },

    { SDL_SCANCODE_HOME,            keyboard::KEY_HOME              },
    { SDL_SCANCODE_PAGEUP,          keyboard::KEY_PAGE_UP           },
    { SDL_SCANCODE_DELETE,          keyboard::KEY_DELETE            },
    { SDL_SCANCODE_END,             keyboard::KEY_END               },
    { SDL_SCANCODE_PAGEDOWN,        keyboard::KEY_PAGE_DOWN         },
    { SDL_SCANCODE_RIGHT,           keyboard::KEY_CURSOR_RIGHT      },
    { SDL_SCANCODE_LEFT,            keyboard::KEY_CURSOR_LEFT       },
    { SDL_SCANCODE_DOWN,            keyboard::KEY_CURSOR_DOWN       },
    { SDL_SCANCODE_UP,              keyboard::KEY_CURSOR_UP         },

    { SDL_SCANCODE_KP_DIVIDE,       keyboard::KEY_NUMPAD_SLASH      },
    { SDL_SCANCODE_KP_MULTIPLY,     keyboard::KEY_NUMPAD_ASTERISK   },
    { SDL_SCANCODE_KP_MINUS,        keyboard::KEY_NUMPAD_MINUS      },
    { SDL_SCANCODE_KP_PLUS,         keyboard::KEY_NUMPAD_PLUS       },
    { SDL_SCANCODE_KP_ENTER,        keyboard::KEY_NUMPAD_ENTER      },
    { SDL_SCANCODE_KP_1,            keyboard::KEY_NUMPAD_1          },
    { SDL_SCANCODE_KP_2,            keyboard::KEY_NUMPAD_2          },
    { SDL_SCANCODE_KP_3,            keyboard::KEY_NUMPAD_3          },
    { SDL_SCANCODE_KP_4,            keyboard::KEY_NUMPAD_4          },
    { SDL_SCANCODE_KP_5,            keyboard::KEY_NUMPAD_5          },
    { SDL_SCANCODE_KP_6,            keyboard::KEY_NUMPAD_6          },
    { SDL_SCANCODE_KP_7,            keyboard::KEY_NUMPAD_7          },
    { SDL_SCANCODE_KP_8,            keyboard::KEY_NUMPAD_8          },
    { SDL_SCANCODE_KP_9,            keyboard::KEY_NUMPAD_9          },
    { SDL_SCANCODE_KP_0,            keyboard::KEY_NUMPAD_0          },
    { SDL_SCANCODE_KP_PERIOD,       keyboard::KEY_NUMPAD_DOT        },

    { SDL_SCANCODE_NONUSBACKSLASH,  keyboard::KEY_LT                },

    { SDL_SCANCODE_LCTRL,           keyboard::KEY_LEFT_CTRL         },
    { SDL_SCANCODE_LSHIFT,          keyboard::KEY_LEFT_SHIFT        },
    { SDL_SCANCODE_LALT,            keyboard::KEY_LEFT_ALT          },

    { SDL_SCANCODE_RCTRL,           keyboard::KEY_RIGHT_CTRL        },
    { SDL_SCANCODE_RSHIFT,          keyboard::KEY_RIGHT_SHIFT       },
    { SDL_SCANCODE_RALT,            keyboard::KEY_RIGHT_ALT         }

#if 0
    SDL_SCANCODE_NONUSHASH = 50,    // ISO USB keyboards actually use this code instead of 49 for the same key
                                    // OSes I've seen treat the two codes identically.
    SDL_SCANCODE_CAPSLOCK = 57,
    SDL_SCANCODE_INSERT = 73,       // Insert on PC, help on some Mac keyboards (but does send code 73, not 117)
    SDL_SCANCODE_NUMLOCKCLEAR = 83, // Num lock on PC, clear on Mac keyboards

    SDL_SCANCODE_LGUI = 227,        // Windows, command (apple), meta
    SDL_SCANCODE_RGUI = 231,        // Windows, command (apple), meta
#endif
};

keyboard::Key to_key(::SDL_Scancode code)
{
    auto it = sdl_to_key.find(code);
    return (it == sdl_to_key.end() ? keyboard::KEY_NONE : it->second);
}

std::string sdl_error()
{
    return ::SDL_GetError();
}

bool in_rect(int x, int y, const ::SDL_Rect& rect)
{
    const int xmax = rect.x + rect.w;
    const int ymax = rect.y + rect.h;
    return ((x >= rect.x && x < xmax) && (y >= rect.y && y < ymax));
}

}
}
}
