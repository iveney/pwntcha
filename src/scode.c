/*
 * scode.c: decode scode captchas
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

#include "config.h"
#include "common.h"

static char find_glyph(struct image *img, int xmin, int xmax);

/* Main function */
char *decode_scode(struct image *img)
{
    char *result;
    int stats[256];
    int x, y, i, incell = 0, cur = 0, xmin = 0;
    int r, g, b;
    struct image *tmp1;

    /* allocate enough place */
    result = malloc(1024 * sizeof(char));

    /* Detect background: first and last 3 lines */
    for(i = 0; i < 256; i++)
        stats[i] = 0;

    for(y = 0; y < 3; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            stats[r]++;
            getpixel(img, x, img->width - 1 - y, &r, &g, &b);
            stats[r]++;
        }

    /* Set non-background colours to 0 */
    tmp1 = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            if(stats[r])
                setpixel(tmp1, x, y, 255, 255, 255);
            else
                setpixel(tmp1, x, y, 0, 0, 0);
        }

    /* Decode glyphs */
    for(x = 0; x < img->width; x++)
    {
        int found = 0;
        for(y = 0; y < img->height; y++)
        {
            getpixel(tmp1, x, y, &r, &g, &b);
            if(!r)
            {
                found = 1;
                break;
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
            /* Find glyph */
            result[cur++] = find_glyph(tmp1, xmin, x);
        }
    }

    image_free(tmp1);

    result[cur] = 0;
    return result;
}

static char find_glyph(struct image *img, int xmin, int xmax)
{
    int ymin = -1, ymax = -1;
    int x, y, count = 0;
    int r, g, b;

    /* Compute vertical bounds of glyph */
    for(y = 0; y < img->height; y++)
    {
        int found = 0;
        for(x = xmin; x < xmax; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            if(!r)
            {
                found = 1;
                break;
            }
        }
        if(found)
        {
            if(ymin == -1)
                ymin = y;
            else
                ymax = y + 1;
        }
    }

    for(x = xmin; x < xmax; x++)
    {
        for(y = ymin; y < ymax; y++)
        {
            getpixel(img, x, y, &r, &g, &b);
            if(!r)
                count += 5 * (y - ymin) ^ 3 * (x - xmin);
                //count += y - ymin;
        }
    }

    switch(count)
    {
        /* Scode font */
        case 778: return '0';
        case 621: return '1';
        case 854: return '2';
        case 784: return '3';
        case 766: return '4';
        case 771: return '5';
        case 976: return '6';
        case 585: return '7';
        case 980: return '8';
        case 896: return '9';
        /* Small font */
        case 584: return '0';
        case 454: return '1';
        case 517: return '2';
        case 447: return '3';
        case 469: return '4';
        case 472: return '5';
        case 564: return '6';
        case 298: return '7';
        case 560: return '8';
        case 536: return '9';
        /* Thin font */
        case 438: return '0';
        case 405: return '1';
        case 485: return '2';
        case 486: return '3';
        case 413: return '4';
        case 509: return '5';
        case 582: return '6';
        case 242: return '7';
        case 579: return '8';
        case 440: return '9';
#if 0
        case 162: return '0';
        case 131: return '1';
        case 150: return '2';
        case 139: return '3';
        case 155: return '4';
        case 159: return '5';
        case 181: return '6';
        case  90: return '7';
        case 180: return '8';
        case 170: return '9';
#endif
        default:
            dprintf("don't know about checksum %i\n", count);
            return '?';
    }
}

