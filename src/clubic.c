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

static struct image *find_glyphs(struct image *img);

/* Our macros */
#define FONTNAME "share/font_clubic.png"
static struct image *font = NULL;
char *result;

/* Main function */
char *decode_clubic(struct image *img)
{
    struct image *tmp1, *tmp2;

    if(!font)
    {
        font = image_load(FONTNAME);
        if(!font)
        {
            fprintf(stderr, "cannot load font %s\n", FONTNAME);
            exit(-1);
        }
    }

    /* clubic captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    tmp1 = filter_equalize(img, 200);
    tmp2 = find_glyphs(tmp1);

    image_free(tmp1);
    image_free(tmp2);

    return result;
}

static struct image *find_glyphs(struct image *img)
{
    char all[] = "0123456789";
    struct
    {
        int xmin, xmax, ymin, ymax;
        int count;
    }
    glyphs[10];
    struct image *dst;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, incell = 0, count = 0, startx = 0, cur = 0;
    int distmin, distx, disty, distch;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, 255, g, 255);
        }

    for(x = 0; x < font->width; x++)
    {
        int found = 0;
        for(y = 0; y < font->height; y++)
        {
            getpixel(font, x, y, &r, &g, &b);
            if(r < 128)
            {
                found = 1;
                count += (255 - r);
            }
        }
        if(found && !incell)
        {
            incell = 1;
            xmin = x;
        }
        else if(!found && incell)
        {
            incell = 0;
            xmax = x;
            ymin = 0;
            ymax = font->height;
            glyphs[i].xmin = xmin;
            glyphs[i].xmax = xmax;
            glyphs[i].ymin = ymin;
            glyphs[i].ymax = ymax;
            glyphs[i].count = count;
            count = 0;
            i++;
        }
    }

    if(i != 10)
    {
        printf("error: could not find 10 glyphs in font\n");
        exit(-1);
    }

    while(cur < 6)
    {
        /* Try to find 1st letter */
        distmin = INT_MAX;
        for(i = 0; i < 10; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = glyphs[i].xmin;
            ymin = glyphs[i].ymin;
            xmax = glyphs[i].xmax;
            ymax = glyphs[i].ymax;
            for(y = -4; y < 4; y++)
                for(x = startx; x < startx + 4; x++)
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
                    dist = dist * 128 / glyphs[i].count;
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
        xmin = glyphs[distch].xmin;
        ymin = glyphs[distch].ymin;
        xmax = glyphs[distch].xmax;
        ymax = glyphs[distch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(font, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128) continue;
                setpixel(dst, distx + x, disty + y, r, g, b);
            }

        startx = distx + xmax - xmin;
        result[cur++] = all[distch];
    }

    return dst;
}

