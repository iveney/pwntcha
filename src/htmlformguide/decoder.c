/*
 * decoder.c: decode html-form-guide.com captchas
 * $Id$
 *
 * contributed by jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
#include <limits.h>

#include "config.h"
#include "common.h"

static void find_glyphs(struct image *img);

/* Our macros */
char *result;

/* Main function */
char *decode_htmlformguide(struct image *img)
{
    struct image *tmp;

    /* htmlformguide.com captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    tmp = image_dup(img);
    filter_contrast(tmp);
    filter_black_stuff(tmp);
    filter_smooth(tmp);
    filter_median(tmp);
    find_glyphs(tmp);

    image_free(tmp);

    return result;
}

static void find_glyphs(struct image *img)
{
#define DELTA 4
    static struct font *font;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, startx = 0, cur = 0;
    int bestdist, bestx, besty, bestch;

    if(!font)
    {
        font = font_load_variable(DECODER, "monofont_40.bmp",
                                  "23456789bcdfghjkmnpqrstvwxyz");
        if(!font)
            exit(1);

        filter_smooth(font->img);
	filter_median(font->img);
    }

    while(cur < 6)
    {
        /* Try to find 1st letter */
        bestdist = INT_MAX;
        for(i = 0; i < font->size; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = font->glyphs[i].xmin - DELTA;
            ymin = font->glyphs[i].ymin;
            xmax = font->glyphs[i].xmax + DELTA;
            ymax = font->glyphs[i].ymax;

            for(y = -3; y < 3; y++)
            {
                for(x = startx; x < startx + 16; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font->img, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            dist += (r - r2) * (r - r2);
                        }
                    dist  = dist / (xmax - xmin - 2 * DELTA);
                    if(dist < localmin)
                    {
                        localmin = dist;
                        localx = x;
                        localy = y;
                    }
                }
            }
            if(localmin < bestdist)
            {
                bestdist = localmin;
                bestx = localx;
                besty = localy;
                bestch = i;
            }
        }

        startx = bestx + font->glyphs[bestch].xmax - font->glyphs[bestch].xmin;
        result[cur++] = font->glyphs[bestch].c;
    }
}

