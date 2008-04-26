/*
 * slashdot.c: decode Slashdot captchas
 * $Id$
 *
 * Copyright: (c) 2004 Sam Hocevar <sam@zoy.org>
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
#include <math.h>

#include "config.h"
#include "common.h"

static void count_objects(struct image *img);
static void rotate(struct image *img);
static void find_glyphs(struct image *img);

/* Global stuff */
struct { int xmin, ymin, xmax, ymax; } objlist[100];
int objects, first, last;
char *result;

/* Main function */
char *decode_slashdot(struct image *img)
{
    struct image *tmp1, *tmp2;

    /* Initialise local data */
    objects = 0;
    first = -1;
    last = -1;

    /* Slashdot captchas have 7 characters */
    result = malloc(8 * sizeof(char));
    strcpy(result, "       ");

    /* Clean image a bit */
    tmp1 = image_dup(img);
    filter_detect_lines(tmp1);
    filter_fill_holes(tmp1);

    /* Detect small objects to guess image orientation */
    tmp2 = image_dup(tmp1);
    filter_median(tmp2);
    filter_threshold(tmp2, 200);
    count_objects(tmp2);

    /* Invert rotation and find glyphs */
    rotate(tmp1);
    filter_median(tmp1);
    find_glyphs(tmp1);

    /* Clean up our mess */
    image_free(tmp1);
    image_free(tmp2);

    /* aaaaaaa means decoding failed */
    if(!strcmp(result, "aaaaaaa"))
        result[0] = '\0';

    return result;
}

/* The following functions are local */

static void count_objects(struct image *img)
{
    struct image *tmp;
    int gotblack = 1;
    int x, y, i;
    int r, g, b;

    tmp = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp, x, y, r, g, b);
        }

    while(gotblack)
    {
        gotblack = 0;
        for(y = 0; y < tmp->height; y++)
            for(x = 0; x < tmp->width; x++)
            {
                getpixel(tmp, x, y, &r, &g, &b);
                if(r == 0 && g == 0 && b == 0)
                {
                    gotblack = 1;
                    filter_flood_fill(tmp, x, y, 254 - objects, 0, 0);
                    objects++;
                }
            }
    }

    //printf("%i objects\n", objects);

    for(i = 0; i < objects; i++)
    {
        objlist[i].ymin = tmp->height;
        objlist[i].ymax = 0;

        for(y = 0; y < tmp->height; y++)
            for(x = 0; x < tmp->width; x++)
            {
                getpixel(tmp, x, y, &r, &g, &b);
                if(r == 255 - i && g == 0 && b == 0)
                {
                    if(y < objlist[i].ymin) { objlist[i].ymin = y; objlist[i].xmin = x; }
                    if(y > objlist[i].ymax) { objlist[i].ymax = y; objlist[i].xmax = x; }
                }
            }
        //printf("y min-max: %i %i (size %i)\n", objlist[i].ymin, objlist[i].ymax, objlist[i].ymax - objlist[i].ymin + 1);
        if(objlist[i].ymax - objlist[i].ymin > 18 && objlist[i].ymax - objlist[i].ymin < 27)
        {
            if(first == -1)
                first = i;
            last = i;
            filter_flood_fill(tmp, objlist[i].xmin, objlist[i].ymin, 0, 0, 255);
        }
    }

    image_swap(img, tmp);
    image_free(tmp);
}

static void rotate(struct image *img)
{
    struct image *tmp;
    int x, y, xdest, ydest;
    int r, g, b;
    //int R, G, B;
    int X = objlist[first].xmin - objlist[last].xmin;
    int Y = objlist[first].ymin - objlist[last].ymin;
    float xtmp, ytmp;
    float sina = (1.0 * Y) / sqrt(1.0 * X * X + Y * Y);
    float cosa = (1.0 * X) / sqrt(1.0 * X * X + Y * Y);
    if(sina * cosa > 0)
    {
        sina = -sina;
        cosa = -cosa;
    }

    tmp = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            xtmp = 1.0 * (x - img->width / 2);
            ytmp = 1.0 * (y - img->height / 2);
            xdest = xtmp * cosa - ytmp * sina + 0.5 * img->width;
            ydest = ytmp * cosa + xtmp * sina + 0.5 * img->height;
            //R = G = B = 0;
            getpixel(img, xdest, ydest, &r, &g, &b);
            //R += r; G += g; B += b;
            //getpixel(img, xdest+1, ydest, &r, &g, &b);
            //R += r; G += g; B += b;
            //getpixel(img, xdest, ydest+1, &r, &g, &b);
            //R += r; G += g; B += b;
            //getpixel(img, xdest+1, ydest+1, &r, &g, &b);
            //R += r; G += g; B += b;
            //r = R / 4; g = G / 4; b = B / 4;
            if(r == 255 && g == 0 && b == 255)
                g = 255;
            setpixel(tmp, x, y, r, g, b);
        }

    image_swap(img, tmp);
    image_free(tmp);
}

static void find_glyphs(struct image *img)
{
    static struct font *font = NULL;
    struct image *tmp;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, startx = 0, cur = 0;
    int distmin, distx, disty, distch;

    if(!font)
    {
        font = font_load_variable(DECODER, "font.png",
                                  "abcdefgijkmnpqrstvwxyz");
        if(!font)
            exit(1);
    }

    tmp = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp, x, y, 255, g, 255);
        }

    while(cur < 7)
    {
        /* Try to find 1st letter */
        distmin = INT_MAX;
        for(i = 0; i < font->size; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = font->glyphs[i].xmin;
            ymin = font->glyphs[i].ymin;
            xmax = font->glyphs[i].xmax;
            ymax = font->glyphs[i].ymax;
            for(y = -5; y < 5; y++)
                for(x = startx - 5; x < startx + 5; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font->img, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            dist += abs(r - r2);
                        }
    //                printf("%i %i -> %i\n", x, y, dist);
                    //dist /= sqrt(xmax - xmin);
                    dist = dist * 128 / font->glyphs[i].count;
                    if(dist < localmin)
                    {
                        localmin = dist;
                        localx = x;
                        localy = y;
                    }
                }
            //fprintf(stderr, "%i (%i,%i)\n", localmin, localx - startx, localy);
            if(localmin < distmin)
            {
                distmin = localmin;
                distx = localx;
                disty = localy;
                distch = i;
            }
        }

        /* Draw best glyph in picture (debugging purposes) */
        xmin = font->glyphs[distch].xmin;
        ymin = font->glyphs[distch].ymin;
        xmax = font->glyphs[distch].xmax;
        ymax = font->glyphs[distch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(font->img, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128) continue;
                setpixel(tmp, distx + x, disty + y, r, g, b);
            }

        startx = distx + xmax - xmin;
        result[cur++] = font->glyphs[distch].c;
    }

    image_swap(img, tmp);
    image_free(tmp);
}

