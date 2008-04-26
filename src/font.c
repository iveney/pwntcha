/*
 * font.c: font handling
 * $Id$
 *
 * Copyright: (c) 2005 Sam Hocevar <sam@zoy.org>
 *  This program is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What The Fuck You Want
 *  To Public License, Version 2, as published by Sam Hocevar. See
 *  http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "common.h"

struct font *font_load_fixed(char const *decoder, char const *file,
                             char const *chars)
{
    char fontname[BUFSIZ];
    struct font *font;
    struct image *img;
    int i;

    sprintf(fontname, "src/%s/%s", decoder, file);
    img = image_load(fontname);
    if(!img)
    {
        sprintf(fontname, "%s/%s/%s", share, decoder, file);
        img = image_load(fontname);
    }
    if(!img)
    {
        fprintf(stderr, "%s: cannot load font %s\n", argv0, fontname);
        return NULL;
    }

    font = malloc(sizeof(struct font));
    font->img = img;
    font->size = strlen(chars);
    font->glyphs = malloc(font->size * sizeof(struct glyph));

    for(i = 0; i < font->size; i++)
    {
        font->glyphs[i].xmin = i * font->img->width / font->size;
        font->glyphs[i].xmax = (i + 1) * font->img->width / font->size;
        font->glyphs[i].ymin = 0;
        font->glyphs[i].ymax = font->img->height;
        font->glyphs[i].count = 0; /* They all have the same width anyway */
        font->glyphs[i].c = chars[i];
    }

    return font;
}

struct font *font_load_variable(char const *decoder, char const *file,
                                char const *chars)
{
    char fontname[BUFSIZ];
    struct font *font;
    struct image *img;
    int count = 0, incell = 0, xmin, xmax, ymin, ymax;
    int x, y, i;
    int r, g, b;

    sprintf(fontname, "src/%s/%s", decoder, file);
    img = image_load(fontname);
    if(!img)
    {
        sprintf(fontname, "%s/%s/%s", share, decoder, file);
        img = image_load(fontname);
    }
    if(!img)
    {
        fprintf(stderr, "%s: cannot load font %s\n", argv0, fontname);
        return NULL;
    }

    font = malloc(sizeof(struct font));
    font->img = img;
    font->size = strlen(chars);
    font->glyphs = malloc(font->size * sizeof(struct glyph));

    for(x = 0, i = 0, xmin = 0; x <= font->img->width && i < font->size; x++)
    {
        int found = 0;
        if(x != font->img->width)
            for(y = 0; y < font->img->height; y++)
            {
                getpixel(font->img, x, y, &r, &g, &b);
                if(r < 250)
                {
                    found = 1;
                    count += (255 - r);
                }
            }
        else
          found = 0;

        if(found && !incell)
        {
            incell = 1;
            xmin = x;
        }
        else if(!found && incell)
        {
            incell = 0;
            xmax = x;
#if 0
            ymin = font->img->height;
            ymax = 0;
            for(y = 0; y < font->img->height; y++)
            {
                int newx;
                int gotit = 0;
                for(newx = xmin; newx < xmax; newx++)
                {
                    getpixel(font->img, newx, y, &r, &g, &b);
                    if(r < 250)
                    {
                        gotit = 1;
                        break;
                    }
                }
                if(gotit)
                {
                    if(ymin > y) ymin = y;
                    if(ymax <= y) ymax = y + 1;
                }
            }
#else
            ymin = 0;
            ymax = font->img->height;
#endif
            font->glyphs[i].xmin = xmin;
            font->glyphs[i].xmax = xmax;
            font->glyphs[i].ymin = ymin;
            font->glyphs[i].ymax = ymax;
            font->glyphs[i].count = count;
            font->glyphs[i].c = chars[i];
            count = 0;
            i++;
        }
    }

    if(i != font->size)
    {
        fprintf(stderr, "%s: could only find %i of %i glyphs in font %s\n",
                argv0, i, font->size, file);
        image_free(font->img);
        free(font->glyphs);
        free(font);
        return NULL;
    }

    return font;
}

void font_free(struct font *font)
{
    image_free(font->img);
    free(font->glyphs);
    free(font);
}

