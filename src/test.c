/*
 * test.c: test captchas
 * $Id$
 *
 * Copyright: (c) 2004 Sam Hocevar <sam@zoy.org>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License as published by Banlu Kemiyatorn. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "config.h"
#include "common.h"

/* Our macros */
#define FONTNAME "share/font_phpbb.png"

static struct image *find_glyphs(struct image *img);

/* Global stuff */
struct { int xmin, ymin, xmax, ymax; } objlist[100];
int objects, first, last;
char *result;

/* Main function */
char *decode_test(struct image *img)
{
    struct image *tmp1, *tmp2, *tmp3, *tmp4, *tmp5, *tmp6, *tmp7;

    /* Initialise local data */
    objects = 0;
    first = -1;
    last = -1;

    /* phpBB captchas have 6 characters */
    result = malloc(7 * sizeof(char));

    tmp1 = filter_smooth(img);
    tmp2 = filter_median(tmp1);
    tmp3 = filter_equalize(tmp2, 130);
    tmp4 = filter_median(tmp3);
    tmp5 = find_glyphs(tmp3);

    image_free(tmp1);
    image_free(tmp2);
    image_free(tmp3);
    image_free(tmp4);
    image_free(tmp5);

    return result;
}

/* The following functions are local */

static struct image *find_glyphs(struct image *img)
{
    char all[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789";
    struct image *dst;
    struct image *font = image_load(FONTNAME);
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, incell = 0, count = 0, cur = 0, offset = -1;
    int distmin, distx, disty, distch;

    if(!font)
    {
        fprintf(stderr, "cannot load font %s\n", FONTNAME);
        exit(-1);
    }

    dst = image_new(img->width, img->height);

    for(x = 0; x < img->width; x++)
        for(y = 0; y < img->height; y++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, 255, g, 255);
            if(r == 0 && offset == -1)
                offset = x;
        }

    strcpy(result, "       ");

    while(cur < 6)
    {
        /* Try to find 1st letter */
        distmin = INT_MAX;
        for(i = 0; i < 35; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = i * 40;
            ymin = 0;
            xmax = i * 40 + 40;
            ymax = 40;
            for(y = 0; y < img->height - 40; y++)
            {
                x = offset - 5;
                if(cur == 0)
                    x -= 15;
                if(x < 0)
                    x = 0;
                for(; x < offset + 10; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            dist += abs(r - r2);
                        }
                    if(dist < localmin)
                    {
                        localmin = dist;
                        localx = x;
                        localy = y;
                    }
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

        /* Print min glyph (debug) */
        xmin = distch * 40;
        ymin = 0;
        xmax = distch * 40 + 40;
        ymax = 40;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(font, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128) continue;
                setpixel(dst, distx + x, disty + y, r, g, b);
            }

        offset = distx + xmax - xmin;
        result[cur++] = all[distch];
    }

    return dst;
}

