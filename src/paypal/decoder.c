/*
 * paypal.c: decode Paypal captchas
 * $Id$
 *
 * modified by jimmikaelkael <jimmikaelkael@wanadoo.fr>
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
char *decode_paypal(struct image *img)
{
    struct image *tmp;

    /* paypal captchas have 8 characters */
    result = malloc(9 * sizeof(char));
    strcpy(result, "        ");

    tmp = image_dup(img);

    /* apply greyscale filter */
    filter_greyscale(tmp);

    /* thresholding */
    filter_threshold(tmp, 30);

    /* further cleaning */
    int x, y, r, g, b;
    for(y = 0; y < tmp->height; y++)
    {
        /* check horizontally to get rid of (horizontal)lines with just a few pixel on */
        int count = 0;
        for(x = 0; x < tmp->width; x++)
        {
            getpixel(tmp, x, y, &r, &g, &b);

            if ((r + g + b) == 0)
            {
                count++;
            }
        }

        if ((count > 0) && (count < 6))
        {
            for(x = 0; x < tmp->width; x++)
            {
                setpixel(tmp, x, y, 255, 255, 255);
            }
        }

        /* get rid of some remaining noise pixels */
        for(x = 0; x < tmp->width; x++)
        {
            getpixel(tmp, x, y, &r, &g, &b);

            if ((x > 0) && (y > 0) && (x < tmp->width-1) && (y < tmp->height-1))
            {
                int ra, rb, rc, rd, re, rf, rg, rh;

                getpixel(tmp, x-1, y-1, &ra, &g, &b);
                getpixel(tmp, x, y-1, &rb, &g, &b);
                getpixel(tmp, x+1, y-1, &rc, &g, &b);
                getpixel(tmp, x-1, y, &rd, &g, &b);
                getpixel(tmp, x+1, y, &re, &g, &b);
                getpixel(tmp, x-1, y+1, &rf, &g, &b);
                getpixel(tmp, x, y+1, &rg, &g, &b);
                getpixel(tmp, x+1, y+1, &rh, &g, &b);

                if ((ra + rb + rc + rd + re + rf + rg + rh) >= 2040)
                {
                    setpixel(tmp, x, y, 255, 255, 255);
                }
            }
        }
    }

    //image_save(tmp, "output.bmp");

    find_glyphs(tmp);

    image_free(tmp);

    return result;
}

static void find_glyphs(struct image *img)
{
#define DELTA 2
    static struct font *font;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, startx = 0, cur = 0;
    int bestdist, bestx, bestch;

    if(!font)
    {
        font = font_load_variable(DECODER, "paypal.bmp",
                                  "23456789ABCDEFGHJKLMNPRSTUVWXYZ");
        if(!font)
            exit(1);
    }

    while(cur < 8)
    {
        /* Try to find 1st letter */
        bestdist = INT_MAX;
        for(i = 0; i < font->size; i++)
        {
            int localmin = INT_MAX, localx;
            xmin = font->glyphs[i].xmin - DELTA;
            ymin = font->glyphs[i].ymin;
            xmax = font->glyphs[i].xmax + DELTA;
            ymax = font->glyphs[i].ymax;

            for(x = startx; x < startx + 18; x++)
            {
                int z, t, dist;
                dist = 0;
                for(t = 0; t < ymax - ymin; t++)
                    for(z = 0; z < xmax - xmin; z++)
                    {
                        int r2;
                        getgray(font->img, xmin + z, ymin + t, &r);
                        getgray(img, x + z, 5 + t, &r2); /* our font mapping is fixed at 5px vertically */
                        dist += (r - r2) * (r - r2);
                    }
                dist  = dist / (xmax - xmin - 2 * DELTA);
                if(dist < localmin)
                {
                    localmin = dist;
                    localx = x;
                }
            }
            if(localmin < bestdist)
            {
                bestdist = localmin;
                bestx = localx;
                bestch = i;
            }
        }

        /* Print min glyph */
#if 0
        xmin = font->glyphs[bestch].xmin - DELTA;
        ymin = font->glyphs[bestch].ymin;
        xmax = font->glyphs[bestch].xmax + DELTA;
        ymax = font->glyphs[bestch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(font->img, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128)
                {
                    getpixel(img, bestx + x, y, &r, &g, &b);
                    r = 255;
                }
                setpixel(img, bestx + x, y, r, g, b);
            }
#endif

        startx = bestx + font->glyphs[bestch].xmax - font->glyphs[bestch].xmin;
        result[cur++] = font->glyphs[bestch].c;
    }
}

