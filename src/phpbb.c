/*
 * phpbb.c: decode phpBB captchas
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

/* Our macros */
#define FONTNAME "font_phpbb.png"
static struct image *font = NULL;

/* Main function */
char *decode_phpbb(struct image *img)
{
    char all[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789";
    char *result;
    struct image *tmp1, *tmp2, *tmp3;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, cur, offset = -1;
    int distmin, distx, disty, distch;

    if(!font)
    {
        char fontname[BUFSIZ];
        sprintf(fontname, "%s/%s", share, FONTNAME);
        font = image_load(fontname);
        if(!font)
        {
            fprintf(stderr, "cannot load font %s\n", fontname);
            exit(-1);
        }
    }

    /* phpBB captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    tmp1 = filter_smooth(img);
    tmp2 = filter_equalize(tmp1, 128);
    tmp3 = image_new(img->width, img->height);

    for(x = 0; x < img->width; x++)
        for(y = 0; y < img->height; y++)
        {
            getpixel(tmp2, x, y, &r, &g, &b);
            if(r == 0 && offset == -1)
                offset = x;
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp3, x, y, 255, g, 255);
        }

    for(cur = 0; cur < 6; cur++)
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
                x = offset - 3;
                if(cur == 0)
                    x -= 10;
                if(x < 0)
                    x = 0;
                for(; x < offset + 3; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font, xmin + z, ymin + t, &r);
                            getgray(tmp2, x + z, y + t, &r2);
                            if(r > r2)
                                dist += r - r2;
                            else
                                dist += (r2 - r) * 3 / 4;
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
                int r2;
                getpixel(font, xmin + x, ymin + y, &r2, &g, &b);
                if(r2 > 128) continue;
                getpixel(tmp3, distx + x, disty + y, &r, &g, &b);
                setpixel(tmp3, distx + x, disty + y, r2, g, b);
            }

        offset = distx + xmax - xmin;
        result[cur] = all[distch];
    }

    image_free(tmp1);
    image_free(tmp2);
    image_free(tmp3);

    return result;
}

