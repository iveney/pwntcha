/*
 * vbulletin.c: decode vbulletin captchas
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

#define FONTNAME "share/font_vbulletin.png"

/* Main function */
char *decode_vbulletin(struct image *img)
{
    char all[] = "2346789ABCDEFGHJKLMNPRTWXYZ";
    char *result;
    struct image *tmp1, *tmp2, *tmp3, *font;
    int limits[6] = { 26, 53, 80, 107, 134, 160 };
    int x, y, r, g, b, i, j;

    font = image_load(FONTNAME);
    if(!font)
    {
        fprintf(stderr, "cannot load font %s\n", FONTNAME);
        exit(-1);
    }

    /* vBulletin captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    /* half the captchas are inverse video; we set them back to normal */
    getpixel(img, 0, 0, &r, &g, &b);
    if(r < 50)
        tmp1 = filter_equalize(img, 128);
    else
        tmp1 = filter_equalize(img, -128);

    /* Remove garbage around the cells */
    for(x = 0; x < img->width; x++)
    {
        for(y = 0; y < 15; y++)
            setpixel(tmp1, x, y, 255, 255, 255);
        for(y = 45; y < img->height; y++)
            setpixel(tmp1, x, y, 255, 255, 255);
    }

    for(x = 0; x < img->width; x++)
    {
        for(i = 0; i < 6; i++)
            if(x == limits[i])
                break;
        if(i == 6)
            for(y = 15; y < 45; y++)
                setpixel(tmp1, x, y, 255, 255, 255);
        else
            x += 11;
    }

    tmp2 = filter_black_stuff(tmp1);
    tmp3 = filter_black_stuff(tmp2);

    /* Fill letters in gray */
    for(x = 26; x < 172; x++)
    {
        getpixel(tmp3, x, 15, &r, &g, &b);
        if(r == 0)
            filter_flood_fill(tmp3, x, 15, 127, 0, 255);
    }

    /* Find remaining black parts and remove them */
    for(x = 26; x < 172; x++)
        for(y = 15; y < 45; y++)
        {
            getpixel(tmp3, x, y, &r, &g, &b);
            if(r == 0)
                filter_flood_fill(tmp3, x, y, 255, 255, 255);
        }

    /* Fill letters in black */
    for(x = 26; x < 172; x++)
    {
        getpixel(tmp3, x, 44, &r, &g, &b);
        if(r == 127)
            filter_flood_fill(tmp3, x, 44, 0, 0, 0);
    }

    /* Find remaining gray parts and remove them */
    for(x = 26; x < 172; x++)
        for(y = 15; y < 45; y++)
        {
            getpixel(tmp3, x, y, &r, &g, &b);
            if(r == 127)
                filter_flood_fill(tmp3, x, y, 255, 255, 255);
        }

    /* Guess all glyphs */
    for(i = 0; i < 6; i++)
    {
        struct image *new;
        int mindist = INT_MAX, min = -1;
        new = filter_crop(tmp3, limits[i], 15, limits[i] + 11, 45);
        for(j = 0; j < 27; j++)
        {
            int dist = 0;
            for(y = 0; y < new->height; y++)
                for(x = 0; x < new->width; x++)
                {
                    int r2, g2, b2;
                    getpixel(font, 12 * j + x, y, &r, &g, &b);
                    getpixel(new, x, y, &r2, &g2, &b2);
                    dist += (r - r2) * (r - r2);
                }
            if(dist < mindist)
            {
                mindist = dist;
                min = j;
            }
        }
        image_free(new);
        result[i] = all[min];
    }

    image_free(tmp1);
    image_free(tmp2);
    image_free(tmp3);
    image_free(font);

    return result;
}

