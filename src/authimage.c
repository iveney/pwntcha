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

#define FONTNAME "font_authimage.png"
static struct image *font = NULL;

/* Main function */
char *decode_authimage(struct image *img)
{
    char *all = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *result;
    struct image *tmp1, *tmp2, *tmp3;
    int x, y, r, g, b, i;

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

    /* authimage captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    memset(result, '\0', 7);

    /* half the captchas are inverse video; we set them back to normal */
    tmp1 = filter_scale(img, 2.0);
    getpixel(img, 0, 0, &r, &g, &b);
    tmp2 = filter_equalize(tmp1, r * 3 / 4);
    tmp3 = filter_smooth(tmp2);

    for(i = 0; i < 6; i++)
    {
        int mindiff = INT_MAX, minch = -1, ch;
        for(ch = 0; ch < 36; ch++)
        {
            int diff = 0;
            for(y = 0; y < 7; y++)
            {
                for(x = 0; x < 5; x++)
                {
                    int newx, newy, r2;
                    newx = 35.0 + (x + 6 * i) * 218.0 / 34.0 + y * 5.0 / 6.0 + 0.5;
                    newy = 33.0 - (x + 6 * i) * 18.0 / 34.0 + y * 42.0 / 6.0 + 0.5;
                    getpixel(tmp3, newx, newy, &r, &g, &b);
                    getpixel(font, x + 6 * ch, y, &r2, &g, &b);
                    r = (r < 220) ? 0 : 255;
                    diff += (r - r2) * (r - r2);
                }
            }
            if(diff < mindiff)
            {
                mindiff = diff;
                minch = ch;
            }
        }
        result[i] = all[minch];
    }

    image_free(tmp3);
    image_free(tmp2);
    image_free(tmp1);

    return result;
}

