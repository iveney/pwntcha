/*
 * xanga.c: decode Xanga captchas
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

static void fill_white_holes(struct image *img);
static void find_glyphs(struct image *img);

static char *result;

/* Main function */
char *decode_xanga(struct image *img)
{
    struct image *tmp;

    /* Xanga captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    tmp = image_dup(img);
image_save(tmp, "xanga1.bmp");
    /* Clean image a bit */
//    filter_equalize(tmp, 200);
    filter_contrast(tmp);
    //filter_detect_lines(tmp);
image_save(tmp, "xanga2.bmp");
    fill_white_holes(tmp);
//    filter_fill_holes(tmp);
    filter_smooth(tmp);
    //filter_median(tmp);
image_save(tmp, "xanga3.bmp");
    //filter_detect_lines(tmp);
//    filter_median(tmp);
//image_save(tmp, "xanga4.bmp");
//    filter_equalize(tmp, 128);
    filter_contrast(tmp);
image_save(tmp, "xanga4.bmp");

#if 0
    /* Detect small objects to guess image orientation */
    filter_median(tmp);
    filter_equalize(tmp, 200);

    /* Invert rotation and find glyphs */
    filter_median(tmp);
#endif
    find_glyphs(tmp);
image_save(tmp, "xanga5.bmp");

    /* Clean up our mess */
    image_free(tmp);

    /* aaaaaaa means decoding failed */
    if(!strcmp(result, "aaaaaaa"))
        result[0] = '\0';

    return result;
}

/* The following functions are local */

static void fill_white_holes(struct image *img)
{
    struct image *tmp;
    int x, y;
    int r, g, b;

    tmp = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp, x, y, r, g, b);
        }

    for(y = 1; y < img->height - 1; y++)
        for(x = 1; x < img->width - 1; x++)
        {
            int count = 0;
            getpixel(img, x, y, &r, &g, &b);
            if(r <= 16)
                continue;
            getpixel(img, x + 1, y, &r, &g, &b);
            count += r;
            getpixel(img, x - 1, y, &r, &g, &b);
            count += r;
            getpixel(img, x, y + 1, &r, &g, &b);
            count += r;
            getpixel(img, x, y - 1, &r, &g, &b);
            count += r;
            if(count > 600)
                continue;
            setpixel(tmp, x, y, count / 5, count / 5, count / 5);
        }

    image_swap(tmp, img);
    image_free(tmp);
}

static void find_glyphs(struct image *img)
{
#define FONTS 6
    static struct font *fonts[FONTS];
    static char *files[] =
    {
        "x_font_freemonobold_32_az.bmp", "abcdefghijklmnopqrstuvwxyz",
        "x_font_freemonobold_24_az.bmp", "abcdefghijklmnopqrstuvwxyz",
        "x_font_freesansbold_32_az.bmp", "abcdefghijklmnopqrstuvwxyz",
        //"x_font_freeserifbold_32_az.bmp", "abcdefghijklmnopqrstuvwxyz",
        "x_font_comic_32_az.bmp", "abcdefghijklmnopqrstuvwxyz",
        "x_font_comic_24_az_messed.bmp", "abcdefghijklmnopqrstuvwxyz",
        "x_font_freesansbold_36_az_messed.bmp", "abcdefghijklmnopqrstuvwxyz",
    };
    struct image *tmp;
    int x, y, i = 0, f;
    int r, g, b;
    int xmin, xmax, ymin, ymax, cur = 0;
    int bestdist, bestfont, bestx, besty, bestch;

    for(f = 0; f < FONTS; f++)
    {
        if(!fonts[f])
        {
            fonts[f] = font_load_variable(files[f * 2], files[f * 2 + 1]);
            if(!fonts[f])
                exit(1);
            //filter_smooth(fonts[f]->img);
            //filter_contrast(fonts[f]->img);
        }
    }

    tmp = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp, x, y, 255, g, 255);
        }

    while(cur < 6)
    {
        /* Try to find 1st letter */
        bestdist = INT_MAX;
        for(f = 0; f < FONTS; f++) for(i = 0; i < fonts[f]->size; i++)
        {
            int localmin = INT_MAX, localx, localy;
int sqr;
            if(fonts[f]->glyphs[i].c == 'l' || fonts[f]->glyphs[i].c == 'z')
                continue;
            xmin = fonts[f]->glyphs[i].xmin - 5;
            ymin = fonts[f]->glyphs[i].ymin - 3;
            xmax = fonts[f]->glyphs[i].xmax + 5;
            ymax = fonts[f]->glyphs[i].ymax + 3;
sqr = sqrt(xmax - xmin);
            for(y = -15; y < 15; y++)
                for(x = 22 - (xmax - xmin) / 2 + 25 * cur; x < 28 - (xmax - xmin) / 2 + 25 * cur; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(fonts[f]->img, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            if(r < r2)
                                dist += (r - r2) * (r - r2);
                            else
                                dist += (r - r2) * (r - r2) * 3 / 4;
                        }
    //                printf("%i %i -> %i\n", x, y, dist);
//                    dist /= (xmax - xmin);
//                    dist = dist / sqrt((ymax - ymin) * (xmax - xmin)) / (xmax - xmin);
                    dist = dist / (xmax - xmin) / sqr;
//                    dist = dist * 128 / fonts[f]->glyphs[i].count;
                    if(dist < localmin)
                    {
                        localmin = dist;
                        localx = x;
                        localy = y;
                    }
                }
            if(localmin < bestdist)
            {
//printf("  bestch is now %i (%c) in font %i\n", i, fonts[f]->glyphs[i].c, f);
                bestdist = localmin;
                bestfont = f;
                bestx = localx;
                besty = localy;
                bestch = i;
            }
        }
//printf("%i (%c) in font %i\n", i, fonts[bestfont]->glyphs[bestch].c, bestfont);
//printf("%i < %i < %i\n", 10 + 25 * cur, bestx, 30 + 25 * cur);

        /* Draw best glyph in picture (debugging purposes) */
        xmin = fonts[bestfont]->glyphs[bestch].xmin - 5;
        ymin = fonts[bestfont]->glyphs[bestch].ymin - 3;
        xmax = fonts[bestfont]->glyphs[bestch].xmax + 5;
        ymax = fonts[bestfont]->glyphs[bestch].ymax + 3;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(fonts[bestfont]->img, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128) continue;
                setpixel(tmp, bestx + x, besty + y, r, g, b);
            }

        result[cur++] = fonts[bestfont]->glyphs[bestch].c;
    }

    image_swap(img, tmp);
    image_free(tmp);
}

