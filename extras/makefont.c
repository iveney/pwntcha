/*
 * makefont.c: create a font bitmap from a .ttf file.
 * $Id$
 *
 * Copyright: (c) 2004 Sam Hocevar <sam@zoy.org>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License as published by Banlu Kemiyatorn. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 *
 * Build example:
 *   gcc -Wall makefont.c -o makefont `sdl-config --cflags --libs` -lSDL_ttf
 *
 * Usage:
 *   makefont <font.ttf> <size> <text> <output.bmp>
 */

#include <stdlib.h>
#include <stdio.h>

#include "SDL.h"
#include "SDL_ttf.h"

int main(int argc, char *argv[])
{
    SDL_Color bg = { 0xff, 0xff, 0xff, 0 };
    SDL_Color fg = { 0x00, 0x00, 0x00, 0 };
    SDL_Surface *text;
    TTF_Font *font;

    if(argc != 5)
    {
        fprintf(stderr, "usage: %s <font.ttf> <size> <text> <output.bmp>\n",
                argv[0]);
        return 1;
    }

    TTF_Init();
    font = TTF_OpenFont(argv[1], atoi(argv[2]));
    if(!font)
    {
        fprintf(stderr, "could not load font %s: %s\n",
                argv[1], SDL_GetError());
        TTF_Quit();
        return 1;
    }

    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    text = TTF_RenderUTF8_Shaded(font, argv[3], fg, bg);
    if(!text)
    {
        fprintf(stderr, "text rendering failed: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        return 1;
    }

    SDL_SaveBMP(text, argv[4]);
    SDL_FreeSurface(text);
    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}

