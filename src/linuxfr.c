/*
 * linuxfr.c: decode linuxfr.org captchas
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

#define FONTNAME "font_linuxfr.png"
static struct image *font = NULL;

/* Main function */
char *decode_linuxfr(struct image *img)
{
    char all[] = "abcdefghijklmnopqrstuvwxyz"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "0123456789";
    char *result;
    struct image *tmp;
    int x, y, r, g, b, i, j, c;
    int *stats = malloc(img->height * sizeof(int));

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

    /* linuxfr captchas have 7 characters */
    result = malloc(8 * sizeof(char));
    memset(result, '\0', 8);

    tmp = filter_equalize(img, 150);

    for(y = 0; y < img->height; y++)
    {
        int count = 0;
        for(x = 0; x < img->width; x++)
        {
            getpixel(tmp, x, y, &r, &g, &b);
            if(r == 0)
                count++;
        }
        stats[y] = count;
    }

    /* Find 7 consecutive lines that have at least 14 pixels; they're
     * baseline candidates */
    for(y = 0; y < img->height - 11; y++)
    {
        int ycan = 1;
        for(j = 3; j < 10; j++)
        {
            if(stats[y + j] < 14)
            {
                ycan = 0;
                y = y + j - 3;
                break;
            }
        }
        if(!ycan)
            continue;

        /* Find 7 consecutive cells that have at least 2 pixels on
         * each line; they're base column candidates */
        for(x = 0; x < img->width - 9 * 7 + 1; x++)
        {
            int goodx = 1;
            for(c = 0; c < 7 && goodx; c++)
            {
                for(j = 3; j < 10; j++)
                {
                    int count = 0;
                    for(i = 0; i < 8; i++)
                    {
                        getpixel(tmp, x + c * 9 + i, y + j, &r, &g, &b);
                        if(r == 0)
                        {
                            count++;
                            if(count == 2)
                                break;
                        }
                    }
                    if(count < 2)
                    {
                        goodx = 0;
                        break;
                    }
                }
            }
            if(!goodx)
                continue;

            /* Now we have an (x,y) candidate - try to fit 7 characters */
            for(c = 0; c < 7 && goodx; c++)
            {
                int r2, g2, b2, ch;
                int minerror = INT_MAX;
                for(ch = 0; ch < 62; ch++)
                {
                    int error = 0, goodch = 1;
                    for(j = 0; j < 12 && goodch; j++)
                        for(i = 0; i < 8; i++)
                        {
                            getpixel(tmp, x + c * 9 + i, y + j, &r, &g, &b);
                            getpixel(font, ch * 9 + i, j, &r2, &g2, &b2);
                            /* Only die if font is black and image is white */
                            if(r > r2)
                            {
                                goodch = 0;
                                break;
                            }
                            else if(r < r2)
                                error++;
                        }
                    if(goodch && error < minerror)
                    {
                        minerror = error;
                        result[c] = all[ch];
                        result[c+1] = '\0';
                    }
                }
                if(minerror == INT_MAX)
                    goodx = 0;
            }
            /* Wow, that was a good guess! Exit this loop */
            if(goodx)
                break;
        }
    }

    image_free(tmp);
    free(stats);

    if(strlen(result) != 7)
    {
        free(result);
        return NULL;
    }

    return result;
}

