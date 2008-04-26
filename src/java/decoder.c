/*
 * java.c: decode java captchas I forgot about
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

/* Main function */
char *decode_java(struct image *img)
{
    struct image *tmp;
    int x, y, dx, dy, best = 0, bestx, besty;
    int r, g, b, r2, g2, b2, r3, g3, b3, r4, g4, b4, i, j, c;

    tmp = image_dup(img);
    filter_threshold(tmp, 245);

    for(dy = 0; dy < 20; dy++)
    {
        if(dy > -5 && dy < 5)
            continue;

        for(dx = -20; dx < 20; dx++)
        {
            int good = 0;

            if(dx > -5 && dx < 5)
                continue;

            for(y = 0; y < tmp->height - dy; y++)
            {
                for(x = 0; x < tmp->width; x++)
                {
                    getpixel(tmp, x, y, &r, &g, &b);
                    getpixel(tmp, x + dx, y + dy, &r2, &g2, &b2);

                    if(r && r2)
                        good++;
                }
            }

            if(good > best)
            {
                best = good;
                bestx = dx;
                besty = dy;
            }
        }
    }

    for(y = 0; y < tmp->height - besty; y++)
    {
        for(x = 0; x < tmp->width; x++)
        {
            getpixel(tmp, x, y, &r, &g, &b);
            getpixel(tmp, x + bestx, y + besty, &r2, &g2, &b2);
            getpixel(tmp, x - bestx, y - besty, &r3, &g3, &b3);
            getpixel(tmp, x + 2 * bestx, y + 2 * besty, &r4, &g4, &b4);

            if(r && r2)
            {
                if(r3 && r4)
                    setpixel(img, x, y, 0, 127, 0);
                else
                    setpixel(img, x, y, 0, 255, 255);
            }
            else if(r)
                setpixel(img, x, y, 0, 0, 127);
            else
                setpixel(img, x, y, 0, 0, 0);
        }
    }

    image_save(img, "test.bmp");
    image_free(tmp);

    return NULL;
}

