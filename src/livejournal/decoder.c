/*
 * livejournal.c: decode livejournal captchas
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
#include <limits.h>

#include "config.h"
#include "common.h"

static void find_glyphs(struct image *img);

/* Our macros */
char *result;

/* Main function */
char *decode_livejournal(struct image *img)
{
    struct image *tmp;

    /* livejournal captchas have 7 characters */
    result = malloc(8 * sizeof(char));
    strcpy(result, "       ");

    tmp = image_dup(img);
    filter_detect_lines(tmp);
    filter_fill_holes(tmp);
    filter_median(tmp);
//    filter_smooth(tmp);
//    filter_contrast(tmp);
    filter_threshold(tmp, 128);
image_save(tmp, "foo.bmp");
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
        font = font_load_variable("x_font_freesansbold_32_09az.bmp",
                                  "0123456789abcdefghijklmnopqrstuvwxyz");
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

    while(cur < 7)
    {
        /* Try to find 1st letter */
        distmin = INT_MAX;
        for(i = 0; i < font->size; i++)
        {
int sqr;
            int localmin = INT_MAX, localx, localy;
            xmin = font->glyphs[i].xmin;
            ymin = font->glyphs[i].ymin;
            xmax = font->glyphs[i].xmax;
            ymax = font->glyphs[i].ymax;
sqr = sqrt(xmax - xmin);
            for(y = -16; y < 8; y++)
                for(x = 25 * cur; x < 25 * cur + 5; x++)
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
                    //dist = dist * 128 / font->glyphs[i].count;
                    dist = dist / (xmax - xmin) / sqr;
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

        startx = distx + 20;
        result[cur++] = font->glyphs[distch].c;
    }

image_save(tmp, "foo2.bmp");
    image_free(tmp);
}

