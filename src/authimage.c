/*
 * authimage.c: decode authimage captchas
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
#include <math.h>

#include "config.h"
#include "common.h"

/* Main function */
char *decode_authimage(struct image *img)
{
    static struct font *font = NULL;
    char *result;
    struct image *tmp;
    int x, y, r, g, b, i;

    if(!font)
    {
        font = font_load_fixed("font_authimage.png",
                               "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if(!font)
            exit(-1);
    }

    /* authimage captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    memset(result, '\0', 7);

    /* double the captcha size for better accuracy in the rotation */
    tmp = image_dup(img);
    filter_scale(tmp, 2.0);
    getpixel(tmp, 0, 0, &r, &g, &b);
    filter_threshold(tmp, r * 3 / 4);
    filter_smooth(tmp);
    filter_threshold(tmp, 220);

    for(i = 0; i < 6; i++)
    {
        int mindiff = INT_MAX, minch = -1, ch;
        for(ch = 0; ch < font->size; ch++)
        {
            int diff = 0;
            for(y = 0; y < 7; y++)
            {
                for(x = 0; x < 5; x++)
                {
                    int newx, newy, r2;
                    newx = 35.0 + (x + 6 * i) * 218.0 / 34.0 + y * 5.0 / 6.0 + 0.5;
                    newy = 33.0 - (x + 6 * i) * 18.0 / 34.0 + y * 42.0 / 6.0 + 0.5;
                    getpixel(tmp, newx, newy, &r, &g, &b);
                    getpixel(font->img, x + 6 * ch, y, &r2, &g, &b);
                    diff += (r - r2) * (r - r2);
                }
            }
            if(diff < mindiff)
            {
                mindiff = diff;
                minch = ch;
            }
        }
        result[i] = font->glyphs[minch].c;
    }

    image_free(tmp);

    return result;
}

