/*
 * paypal.c: decode Paypal captchas
 * $Id$
 *
 * Copyright: (c) 2005 Sam Hocevar <sam@zoy.org>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License as published by Banlu Kemiyatorn. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
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
char *decode_paypal(struct image *img)
{
    struct image *tmp;

    /* paypal captchas have 8 characters */
    result = malloc(9 * sizeof(char));
    strcpy(result, "        ");

    tmp = image_dup(img);
    find_glyphs(tmp);

    image_free(tmp);

    return result;
}

static void find_glyphs(struct image *img)
{
#define DELTA 2
#define FONTS 2
    static struct font *fonts[FONTS];
    static char *files[] =
    {
        "font_stencil_23_AZ.bmp", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "font_stencil_24_AZ.bmp", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    };
    int x, y, i = 0, f;
    int r, g, b;
    int xmin, xmax, ymin, ymax, startx = 0, cur = 0;
    int bestdist, bestfont, bestx, besty, bestch;

    for(f = 0; f < FONTS; f++)
    {
        if(!fonts[f])
        {
            fonts[f] = font_load_variable(files[f * 2], files[f * 2 + 1]);
            if(!fonts[f])
                exit(1);
        }
    }

    while(cur < 8)
    {
        /* Try to find 1st letter */
        bestdist = INT_MAX;
        for(f = 0; f < FONTS; f++) for(i = 0; i < fonts[f]->size; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = fonts[f]->glyphs[i].xmin - DELTA;
            ymin = fonts[f]->glyphs[i].ymin;
            xmax = fonts[f]->glyphs[i].xmax + DELTA;
            ymax = fonts[f]->glyphs[i].ymax;
            for(y = -3; y < 1; y++)
            {
                for(x = startx; x < startx + 15; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(fonts[f]->img, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            if(r < r2)
                                dist += abs(r - r2);
                            else
                                dist += abs(r - r2) * 3 / 4;
                        }
                    //dist = dist * 128 / fonts[f]->glyphs[i].count;
                    dist  = dist * 1024 / (xmax - xmin - 2 * DELTA);
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
                bestfont = f;
                bestx = localx;
                besty = localy;
                bestch = i;
            }
        }

        /* Print min glyph */
#if 0
        xmin = fonts[bestfont]->glyphs[bestch].xmin - DELTA;
        ymin = fonts[bestfont]->glyphs[bestch].ymin;
        xmax = fonts[bestfont]->glyphs[bestch].xmax + DELTA;
        ymax = fonts[bestfont]->glyphs[bestch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(fonts[bestfont]->img, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128)
                    continue;
                setpixel(img, bestx + x, besty + y, r, g, b);
            }
#endif

        startx = bestx + xmax - xmin;
        result[cur++] = fonts[bestfont]->glyphs[bestch].c;
    }
}

