/*
 * filters.c: various image filters
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
#include <math.h>

#include "config.h"
#include "common.h"

/* Functions */
void filter_flood_fill(struct image *img, int x, int y, int r, int g, int b)
{
    int oldr, oldg, oldb;
    int nextr, nextg, nextb;

    if(x < 0 || y < 0 || x >= img->width || y >= img->height)
        return;

    getpixel(img, x, y, &oldr, &oldg, &oldb);
    setpixel(img, x, y, r, g, b);

    getpixel(img, x + 1, y, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        filter_flood_fill(img, x + 1, y, r, g, b);

    getpixel(img, x - 1, y, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        filter_flood_fill(img, x - 1, y, r, g, b);

    getpixel(img, x, y + 1, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        filter_flood_fill(img, x, y + 1, r, g, b);

    getpixel(img, x, y - 1, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        filter_flood_fill(img, x, y - 1, r, g, b);
}

struct image *filter_dup(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    return dst;
}

struct image *filter_scale(struct image *img, float ratio)
{
    struct image *dst;
    int w, h, x, y;
    int r, g, b;

    w = ratio * img->width;
    h = ratio * img->height;

    dst = image_new(w, h);

    for(y = 0; y < h; y++)
        for(x = 0; x < w; x++)
        {
            getpixel(img, x / ratio, y / ratio, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    return dst;
}

struct image *filter_fill_holes(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    for(y = 0; y < dst->height; y++)
        for(x = 2; x < dst->width - 2; x++)
        {
            int c1, c2, c3, c4, c5;
            getpixel(img, x-2, y, &c1, &g, &b);
            getpixel(img, x-1, y, &c2, &g, &b);
            getpixel(img, x, y, &c3, &g, &b);
            getpixel(img, x+1, y, &c4, &g, &b);
            getpixel(img, x+2, y, &c5, &g, &b);
            if(c1 < 127 && c2 < 127 && c3 > 128 && c4 < 127)
                c3 = (c1 + c2 + c4) / 3;
            else if(c2 < 127 && c3 > 128 && c4 < 127 && c5 < 127)
                c3 = (c2 + c4 + c5) / 3;
            setpixel(dst, x, y, c3, c3, c3);
        }

    for(x = 0; x < dst->width; x++)
        for(y = 2; y < dst->height - 2; y++)
        {
            int c1, c2, c3, c4, c5;
            getpixel(img, x, y-2, &c1, &g, &b);
            getpixel(img, x, y-1, &c2, &g, &b);
            getpixel(img, x, y, &c3, &g, &b);
            getpixel(img, x, y+1, &c4, &g, &b);
            getpixel(img, x, y+2, &c5, &g, &b);
            if(c1 < 127 && c2 < 127 && c3 > 128 && c4 < 127)
                c3 = (c1 + c2 + c4) / 3;
            else if(c2 < 127 && c3 > 128 && c4 < 127 && c5 < 127)
                c3 = (c2 + c4 + c5) / 3;
            setpixel(dst, x, y, c3, c3, c3);
        }

    return dst;
}

struct image *filter_black_stuff(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, ra, rb, g, b;

    dst = image_new(img->width, img->height);

    /* Remove vertical stuff */
    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
            if(y > 0 && y < img->height - 1)
            {
                getpixel(img, x, y - 1, &ra, &g, &b);
                getpixel(img, x, y + 1, &rb, &g, &b);
                if(r < ra && (r - ra) * (r - rb) > 5000)
                    setpixel(dst, x, y, ra, ra, ra);
            }
        }

    /* Remove horizontal stuff */
    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            if(x > 0 && x < img->width - 1)
            {
                getpixel(dst, x - 1, y, &ra, &g, &b);
                getpixel(dst, x + 1, y, &rb, &g, &b);
                if(r < ra && (r - ra) * (r - rb) > 5000)
                    setpixel(dst, x, y, ra, ra, ra);
            }
        }

    return dst;
}

struct image *filter_detect_lines(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, ra, rb, g, b;

    dst = image_new(img->width, img->height);

    /* Remove white lines */
    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
            if(y > 0 && y < img->height - 1)
            {
                getpixel(img, x, y - 1, &ra, &g, &b);
                getpixel(img, x, y + 1, &rb, &g, &b);
                if(r > ra && (r - ra) * (r - rb) > 5000)
                    setpixel(dst, x, y, ra, ra, ra);
            }
        }

    /* Remove black lines */
    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(dst, x, y, &r, &g, &b);
            if(y > 0 && y < img->height - 1)
            {
                getpixel(img, x, y - 1, &ra, &g, &b);
                getpixel(img, x, y + 1, &rb, &g, &b);
                if(r < ra && (r - ra) * (r - rb) > 500)
                    setpixel(dst, x, y, ra, ra, ra);
            }
        }

    return dst;
}

struct image *filter_equalize(struct image *img, int threshold)
{
    struct image *dst;
    int x, y;
    int r, g, b;
    int min = 0, max = 255;

    dst = image_new(img->width, img->height);

    if(threshold < 0)
    {
        min = 255;
        max = 0;
        threshold = -threshold;
    }

    threshold *= 3;

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            if(r + g + b < threshold)
                setpixel(dst, x, y, min, min, min);
            else
                setpixel(dst, x, y, max, max, max);
        }

    return dst;
}

struct image *filter_trick(struct image *img)
{
#define TSIZE 3
    struct image *dst;
    int x, y, i, j, val, m, more, l, less;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
            setpixel(dst, x, y, 255, 255, 255);

    for(y = TSIZE/2; y < img->height - TSIZE/2; y++)
        for(x = TSIZE/2; x < img->width - TSIZE/2; x++)
        {
            getpixel(img, x + TSIZE - TSIZE/2, y + TSIZE - TSIZE/2, &val, &g, &b);
            m = more = l = less = 0;
            for(i = 0; i < TSIZE; i++)
                for(j = 0; j < TSIZE; j++)
                {
                    getpixel(img, x + j - TSIZE/2, y + i - TSIZE/2, &r, &g, &b);
                    if(r > val)
                    {
                        more += r;
                        m++;
                    }
                    else if(r < val)
                    {
                        less += r;
                        l++;
                    }
                }

            if(l >= 6)
                i = less / l;
            else if(m >= 6)
                i = more / m;
            else
                i = val;
            setpixel(dst, x, y, i, i, i);
        }

    return dst;
}

struct image *filter_smooth(struct image *img)
{
#define SSIZE 3
    struct image *dst;
    int x, y, i, j, val;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
            setpixel(dst, x, y, 255, 255, 255);

    for(y = SSIZE/2; y < img->height - SSIZE/2; y++)
        for(x = SSIZE/2; x < img->width - SSIZE/2; x++)
        {
            val = 0;
            for(i = 0; i < SSIZE; i++)
                for(j = 0; j < SSIZE; j++)
                {
                    getpixel(img, x + j - SSIZE/2, y + i - SSIZE/2, &r, &g, &b);
                    val += r;
                }

            i = val / (SSIZE * SSIZE);
            setpixel(dst, x, y, i, i, i);
        }

    return dst;
}

struct image *filter_median(struct image *img)
{
#define MSIZE 3
    struct image *dst;
    int x, y, i, j, val[MSIZE*MSIZE];
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
            setpixel(dst, x, y, 255, 255, 255);

    for(y = MSIZE/2; y < img->height - MSIZE/2; y++)
        for(x = MSIZE/2; x < img->width - MSIZE/2; x++)
        {
            for(i = 0; i < MSIZE; i++)
                for(j = 0; j < MSIZE; j++)
                {
                    getpixel(img, x + j - SSIZE/2, y + i - SSIZE/2, &r, &g, &b);
                    val[i * MSIZE + j] = r;
                }

            /* Bubble sort power! */
            for(i = 0; i < MSIZE * MSIZE / 2 + 1; i++)
                for(j = i + 1; j < MSIZE * MSIZE; j++)
                    if(val[i] > val[j])
                    {
                        register int k = val[i];
                        val[i] = val[j];
                        val[j] = k;
                    }

            i = val[MSIZE * MSIZE / 2];
            setpixel(dst, x, y, i, i, i);
        }

    return dst;
}

struct image *filter_contrast(struct image *img)
{
    struct image *dst;
    int histo[256];
    int x, y, i, min = 255, max = 0;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getgray(img, x, y, &r);
            if(r < min) min = r;
            if(r > max) max = r;
        }

    if(min == max)
        histo[min] = 127;
    else
        for(i = min; i < max; i++)
            histo[i] = (i - min) * 255 / (max - min);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getgray(img, x, y, &r);
            setpixel(dst, x, y, histo[r], histo[r], histo[r]);
        }

    return dst;
}

struct image *filter_crop(struct image *img,
                          int xmin, int ymin, int xmax, int ymax)
{
    struct image *dst;
    int x, y;
    int r, g, b;

    if(xmin < 0)
        xmin = 0;
    if(ymin < 0)
        ymin = 0;
    if(xmax >= img->width)
        xmax = img->width - 1;
    if(ymax >= img->height)
        ymax = img->height - 1;

    if(xmin >= xmax || ymin >= ymax)
        return NULL;

    dst = image_new(xmax - xmin, ymax - ymin);

    for(y = 0; y < dst->height; y++)
        for(x = 0; x < dst->width; x++)
        {
            getpixel(img, xmin + x, ymin + y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    return dst;
}

int filter_count(struct image *img)
{
    int histo[256];
    int x, y, i, count = 0;
    int r, g, b;

    for(i = 0; i < 256; i++)
        histo[i] = 0;

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getgray(img, x, y, &r);
            histo[r] = 1;
        }

    for(i = 0; i < 256; i++)
        count += histo[i];

    return count;
}

