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

static struct image *fill_white_holes(struct image *img);
static struct image *rotate(struct image *img);
static struct image *cut_cells(struct image *img);
static struct image *find_glyphs(struct image *img);

/* Our macros */
#define FACTOR 1
#define FONTNAME "font_xanga.png" // use with FACTOR = 1
//#define FONTNAME "font.png" // use with FACTOR = 2
//#define FONTNAME "font_dilated.png" // use with FACTOR = 2
static struct image *font = NULL;

/* Global stuff */
struct { int xmin, ymin, xmax, ymax; } objlist[100];
int objects, first, last;
char *result;

/* Main function */
char *decode_xanga(struct image *img)
{
    struct image *tmp1, *tmp2, *tmp3, *tmp4, *tmp5, *tmp6, *tmp7;

    /* Initialise local data */
    objects = 0;
    first = -1;
    last = -1;

    /* Xanga captchas have 7 characters */
    result = malloc(8 * sizeof(char));
    strcpy(result, "       ");

image_save(img, "xanga1.bmp");
    /* Clean image a bit */
//    tmp1 = filter_equalize(img, 200);
    tmp1 = filter_contrast(img);
    //tmp1 = filter_detect_lines(img);
image_save(tmp1, "xanga2.bmp");
    tmp2 = fill_white_holes(tmp1);
//    tmp2 = filter_fill_holes(tmp1);
image_save(tmp2, "xanga3.bmp");
    //tmp3 = filter_detect_lines(tmp2);
//    tmp3 = filter_median(tmp2);
//image_save(tmp3, "xanga4.bmp");
    tmp3 = filter_equalize(tmp2, 128);
image_save(tmp3, "xanga4.bmp");
return NULL;

    /* Detect small objects to guess image orientation */
    tmp3 = filter_median(tmp2);
    tmp4 = filter_equalize(tmp3, 200);

    /* Invert rotation and find glyphs */
    tmp5 = rotate(tmp2);
    tmp6 = filter_median(tmp5);

    /* Clean up our mess */
    image_free(tmp1);
    image_free(tmp2);
    image_free(tmp3);
    image_free(tmp4);
    image_free(tmp5);
    image_free(tmp6);
    image_free(tmp7);

    /* aaaaaaa means decoding failed */
    if(!strcmp(result, "aaaaaaa"))
        result[0] = '\0';

    return result;
}

/* The following functions are local */

static struct image *fill_white_holes(struct image *img)
{
    struct image *dst;
    int x, y, i;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
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
            setpixel(dst, x, y, count / 5, count / 5, count / 5);
        }

    return dst;
}

