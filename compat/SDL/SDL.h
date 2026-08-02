/*
 * SDL1 -> SDL2 compatibility shim for Kuklomenos OS/2 port.
 * Include path: -Icompat so that <SDL/SDL.h> resolves here.
 */
#ifndef SDL_COMPAT_OS2_H
#define SDL_COMPAT_OS2_H

#include <SDL2/SDL.h>

/* ---- Type aliases ---- */
typedef SDL_Keycode   SDLKey;
typedef SDL_Keymod    SDLMod;
typedef SDL_Keysym    SDL_keysym;   /* SDL1 used lowercase */

/* ---- Renamed SDLK constants ---- */
#define SDLK_KP0          SDLK_KP_0
#define SDLK_KP1          SDLK_KP_1
#define SDLK_KP2          SDLK_KP_2
#define SDLK_KP3          SDLK_KP_3
#define SDLK_KP4          SDLK_KP_4
#define SDLK_KP5          SDLK_KP_5
#define SDLK_KP6          SDLK_KP_6
#define SDLK_KP7          SDLK_KP_7
#define SDLK_KP8          SDLK_KP_8
#define SDLK_KP9          SDLK_KP_9
#define SDLK_NUMLOCK      SDLK_NUMLOCKCLEAR
#define SDLK_SCROLLOCK    SDLK_SCROLLLOCK
#define SDLK_CAPSLOCK     SDLK_CAPSLOCK    /* same name */
#define SDLK_RMETA        SDLK_RGUI
#define SDLK_LMETA        SDLK_LGUI
#define SDLK_LSUPER       SDLK_LGUI
#define SDLK_RSUPER       SDLK_RGUI
#define SDLK_COMPOSE      SDLK_APPLICATION
#define SDLK_PRINT        SDLK_PRINTSCREEN
#define SDLK_BREAK        SDLK_STOP
#define SDLK_EURO         SDLK_UNKNOWN
#define SDLK_UNDO         SDLK_UNKNOWN

/* SDL1 had SDLK_EXCLAIM etc. for punctuation — SDL2 uses ASCII values */
#ifndef SDLK_EXCLAIM
#define SDLK_EXCLAIM      '!'
#define SDLK_QUOTEDBL     '"'
#define SDLK_HASH         '#'
#define SDLK_DOLLAR       '$'
#define SDLK_AMPERSAND    '&'
#define SDLK_QUOTE        '\''
#define SDLK_LEFTPAREN    '('
#define SDLK_RIGHTPAREN   ')'
#define SDLK_ASTERISK     '*'
#define SDLK_PLUS         '+'
#define SDLK_COMMA        ','
#define SDLK_MINUS        '-'
#define SDLK_PERIOD       '.'
#define SDLK_SLASH        '/'
#define SDLK_COLON        ':'
#define SDLK_SEMICOLON    ';'
#define SDLK_LESS         '<'
#define SDLK_EQUALS       '='
#define SDLK_GREATER      '>'
#define SDLK_QUESTION     '?'
#define SDLK_AT          '@'
#define SDLK_LEFTBRACKET  '['
#define SDLK_BACKSLASH    '\\'
#define SDLK_RIGHTBRACKET ']'
#define SDLK_CARET        '^'
#define SDLK_UNDERSCORE   '_'
#define SDLK_BACKQUOTE    '`'
#endif

/* SDL1 WORLD keys — unmapped on OS/2, treat as unknown */
#define SDLK_WORLD_0   SDLK_UNKNOWN
#define SDLK_WORLD_1   SDLK_UNKNOWN
#define SDLK_WORLD_2   SDLK_UNKNOWN
#define SDLK_WORLD_3   SDLK_UNKNOWN
#define SDLK_WORLD_4   SDLK_UNKNOWN
#define SDLK_WORLD_5   SDLK_UNKNOWN
#define SDLK_WORLD_6   SDLK_UNKNOWN
#define SDLK_WORLD_7   SDLK_UNKNOWN
#define SDLK_WORLD_8   SDLK_UNKNOWN
#define SDLK_WORLD_9   SDLK_UNKNOWN
#define SDLK_WORLD_10  SDLK_UNKNOWN
#define SDLK_WORLD_11  SDLK_UNKNOWN
#define SDLK_WORLD_12  SDLK_UNKNOWN
#define SDLK_WORLD_13  SDLK_UNKNOWN
#define SDLK_WORLD_14  SDLK_UNKNOWN
#define SDLK_WORLD_15  SDLK_UNKNOWN
#define SDLK_WORLD_16  SDLK_UNKNOWN
#define SDLK_WORLD_17  SDLK_UNKNOWN
#define SDLK_WORLD_18  SDLK_UNKNOWN
#define SDLK_WORLD_19  SDLK_UNKNOWN
#define SDLK_WORLD_20  SDLK_UNKNOWN
#define SDLK_WORLD_21  SDLK_UNKNOWN
#define SDLK_WORLD_22  SDLK_UNKNOWN
#define SDLK_WORLD_23  SDLK_UNKNOWN
#define SDLK_WORLD_24  SDLK_UNKNOWN
#define SDLK_WORLD_25  SDLK_UNKNOWN
#define SDLK_WORLD_26  SDLK_UNKNOWN
#define SDLK_WORLD_27  SDLK_UNKNOWN
#define SDLK_WORLD_28  SDLK_UNKNOWN
#define SDLK_WORLD_29  SDLK_UNKNOWN
#define SDLK_WORLD_30  SDLK_UNKNOWN
#define SDLK_WORLD_31  SDLK_UNKNOWN
#define SDLK_WORLD_32  SDLK_UNKNOWN
#define SDLK_WORLD_33  SDLK_UNKNOWN
#define SDLK_WORLD_34  SDLK_UNKNOWN
#define SDLK_WORLD_35  SDLK_UNKNOWN
#define SDLK_WORLD_36  SDLK_UNKNOWN
#define SDLK_WORLD_37  SDLK_UNKNOWN
#define SDLK_WORLD_38  SDLK_UNKNOWN
#define SDLK_WORLD_39  SDLK_UNKNOWN
#define SDLK_WORLD_40  SDLK_UNKNOWN
#define SDLK_WORLD_41  SDLK_UNKNOWN
#define SDLK_WORLD_42  SDLK_UNKNOWN
#define SDLK_WORLD_43  SDLK_UNKNOWN
#define SDLK_WORLD_44  SDLK_UNKNOWN
#define SDLK_WORLD_45  SDLK_UNKNOWN
#define SDLK_WORLD_46  SDLK_UNKNOWN
#define SDLK_WORLD_47  SDLK_UNKNOWN
#define SDLK_WORLD_48  SDLK_UNKNOWN
#define SDLK_WORLD_49  SDLK_UNKNOWN
#define SDLK_WORLD_50  SDLK_UNKNOWN
#define SDLK_WORLD_51  SDLK_UNKNOWN
#define SDLK_WORLD_52  SDLK_UNKNOWN
#define SDLK_WORLD_53  SDLK_UNKNOWN
#define SDLK_WORLD_54  SDLK_UNKNOWN
#define SDLK_WORLD_55  SDLK_UNKNOWN
#define SDLK_WORLD_56  SDLK_UNKNOWN
#define SDLK_WORLD_57  SDLK_UNKNOWN
#define SDLK_WORLD_58  SDLK_UNKNOWN
#define SDLK_WORLD_59  SDLK_UNKNOWN
#define SDLK_WORLD_60  SDLK_UNKNOWN
#define SDLK_WORLD_61  SDLK_UNKNOWN
#define SDLK_WORLD_62  SDLK_UNKNOWN
#define SDLK_WORLD_63  SDLK_UNKNOWN
#define SDLK_WORLD_64  SDLK_UNKNOWN
#define SDLK_WORLD_65  SDLK_UNKNOWN
#define SDLK_WORLD_66  SDLK_UNKNOWN
#define SDLK_WORLD_67  SDLK_UNKNOWN
#define SDLK_WORLD_68  SDLK_UNKNOWN
#define SDLK_WORLD_69  SDLK_UNKNOWN
#define SDLK_WORLD_70  SDLK_UNKNOWN
#define SDLK_WORLD_71  SDLK_UNKNOWN
#define SDLK_WORLD_72  SDLK_UNKNOWN
#define SDLK_WORLD_73  SDLK_UNKNOWN
#define SDLK_WORLD_74  SDLK_UNKNOWN
#define SDLK_WORLD_75  SDLK_UNKNOWN
#define SDLK_WORLD_76  SDLK_UNKNOWN
#define SDLK_WORLD_77  SDLK_UNKNOWN
#define SDLK_WORLD_78  SDLK_UNKNOWN
#define SDLK_WORLD_79  SDLK_UNKNOWN
#define SDLK_WORLD_80  SDLK_UNKNOWN
#define SDLK_WORLD_81  SDLK_UNKNOWN
#define SDLK_WORLD_82  SDLK_UNKNOWN
#define SDLK_WORLD_83  SDLK_UNKNOWN
#define SDLK_WORLD_84  SDLK_UNKNOWN
#define SDLK_WORLD_85  SDLK_UNKNOWN
#define SDLK_WORLD_86  SDLK_UNKNOWN
#define SDLK_WORLD_87  SDLK_UNKNOWN
#define SDLK_WORLD_88  SDLK_UNKNOWN
#define SDLK_WORLD_89  SDLK_UNKNOWN
#define SDLK_WORLD_90  SDLK_UNKNOWN
#define SDLK_WORLD_91  SDLK_UNKNOWN
#define SDLK_WORLD_92  SDLK_UNKNOWN
#define SDLK_WORLD_93  SDLK_UNKNOWN
#define SDLK_WORLD_94  SDLK_UNKNOWN
#define SDLK_WORLD_95  SDLK_UNKNOWN

/* ---- Modifier key aliases ---- */
#define KMOD_META  KMOD_GUI

/* ---- Removed/renamed SDL1 video surface flags ---- */
#define SDL_SWSURFACE  0
#define SDL_HWSURFACE  0
#define SDL_ANYFORMAT  0
#define SDL_RESIZABLE  0
#define SDL_FULLSCREEN 0   /* fullscreen not supported in this OS/2 port */
#define SDL_HWPALETTE  0
#define SDL_SRCALPHA   0   /* ignored in SDL2; CreateRGBSurface flags are no-ops */

/* SDL1 SDL_SetAlpha -> SDL2 blend mode API */
static inline int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
    (void)flag;
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceAlphaMod(surface, alpha);
    return 0;
}

/* ---- Removed SDL1 functions ---- */
#define SDL_EnableKeyRepeat(delay, interval) ((void)0)
#define SDL_DEFAULT_REPEAT_DELAY    500
#define SDL_DEFAULT_REPEAT_INTERVAL  30

#endif /* SDL_COMPAT_OS2_H */
