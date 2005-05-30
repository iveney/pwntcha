/*
 * clubic.c: decode clubic captchas
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
char *decode_clubic(struct image *img)
{
    struct image *tmp;

    /* clubic captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    tmp = image_dup(img);
    filter_threshold(tmp, 200);
    find_glyphs(tmp);

    image_free(tmp);

    return result;
}

static void find_glyphs(struct image *img)
{
    static struct font *font = NULL;
    struct image *tmp;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, startx = 0, cur = 0;
    int distmin, distx, disty, distch;

    if(!font)
    {
        font = font_load_variable("font_clubic.png", "0123456789");
        if(!font)
            exit(1);
    }

    tmp = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp, x, y, 255, g, 255);
        }

    while(cur < 6)
    {
        /* Try to find 1st letter */
        distmin = INT_MAX;
        for(i = 0; i < font->size; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = font->glyphs[i].xmin;
            ymin = font->glyphs[i].ymin;
            xmax = font->glyphs[i].xmax;
            ymax = font->glyphs[i].ymax;
            for(y = -4; y < 4; y++)
                for(x = startx; x < startx + 4; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font->img, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            dist += abs(r - r2);
                        }
                    dist = dist * 128 / font->glyphs[i].count;
                    if(dist < localmin)
                    {
                        localmin = dist;
                        localx = x;
                        localy = y;
                    }
                }
            if(localmin < distmin)
            {
                distmin = localmin;
                distx = localx;
                disty = localy;
                distch = i;
            }
        }

        /* Print min glyph */
        xmin = font->glyphs[distch].xmin;
        ymin = font->glyphs[distch].ymin;
        xmax = font->glyphs[distch].xmax;
        ymax = font->glyphs[distch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(font->img, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128)
                    continue;
                setpixel(tmp, distx + x, disty + y, r, g, b);
            }

        startx = distx + xmax - xmin;
        result[cur++] = font->glyphs[distch].c;
    }

    image_free(tmp);
}

