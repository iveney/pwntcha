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

#define FONTNAME "font_vbulletin.png"
static struct image *font = NULL;

/* Main function */
char *decode_vbulletin(struct image *img)
{
    char all[] = "2346789ABCDEFGHJKLMNPRTWXYZ";
    char *result;
    struct image *tmp;
    int limits[6] = { 26, 53, 80, 107, 134, 160 };
    int x, y, r, g, b, i, j;

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

    /* vBulletin captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    /* half the captchas are inverse video; we set them back to normal */
    tmp = image_dup(img);
    getpixel(tmp, 0, 0, &r, &g, &b);
    if(r < 50)
        filter_equalize(tmp, 128);
    else
        filter_equalize(tmp, -128);

    /* Remove garbage around the cells */
    for(x = 0; x < tmp->width; x++)
    {
        for(y = 0; y < 15; y++)
            setpixel(tmp, x, y, 255, 255, 255);
        for(y = 45; y < tmp->height; y++)
            setpixel(tmp, x, y, 255, 255, 255);
    }

    for(x = 0; x < tmp->width; x++)
    {
        for(i = 0; i < 6; i++)
            if(x == limits[i])
                break;
        if(i == 6)
            for(y = 15; y < 45; y++)
                setpixel(tmp, x, y, 255, 255, 255);
        else
            x += 11;
    }

    filter_black_stuff(tmp);
    filter_black_stuff(tmp);

    /* Fill letters in gray */
    for(x = 26; x < 172; x++)
    {
        getpixel(tmp, x, 15, &r, &g, &b);
        if(r == 0)
            filter_flood_fill(tmp, x, 15, 127, 0, 255);
    }

    /* Find remaining black parts and remove them */
    for(x = 26; x < 172; x++)
        for(y = 15; y < 45; y++)
        {
            getpixel(tmp, x, y, &r, &g, &b);
            if(r == 0)
                filter_flood_fill(tmp, x, y, 255, 255, 255);
        }

    /* Fill letters in black */
    for(x = 26; x < 172; x++)
    {
        getpixel(tmp, x, 44, &r, &g, &b);
        if(r == 127)
            filter_flood_fill(tmp, x, 44, 0, 0, 0);
    }

    /* Find remaining gray parts and remove them */
    for(x = 26; x < 172; x++)
        for(y = 15; y < 45; y++)
        {
            getpixel(tmp, x, y, &r, &g, &b);
            if(r == 127)
                filter_flood_fill(tmp, x, y, 255, 255, 255);
        }

    /* Guess all glyphs */
    for(i = 0; i < 6; i++)
    {
        struct image *new = image_dup(tmp);
        int mindist = INT_MAX, min = -1;
        filter_crop(new, limits[i], 15, limits[i] + 11, 45);
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

    image_free(tmp);

    return result;
}

