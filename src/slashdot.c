/*
 * slashdot.c: decode Slashdot captchas
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

/* Our macros */
#define FACTOR 1
#define FONTNAME "share/font_slashdot.png" // use with FACTOR = 1
//#define FONTNAME "share/font.png" // use with FACTOR = 2
//#define FONTNAME "share/font_dilated.png" // use with FACTOR = 2

static struct image *count_objects(struct image *img);
static struct image *rotate(struct image *img);
static struct image *cut_cells(struct image *img);
static struct image *find_glyphs(struct image *img);

/* Global stuff */
struct { int xmin, ymin, xmax, ymax; } objlist[100];
int objects, first, last;
char *result;

/* Main function */
char *decode_slashdot(struct image *img)
{
    struct image *tmp1, *tmp2, *tmp3, *tmp4, *tmp5, *tmp6, *tmp7;

    /* Initialise local data */
    objects = 0;
    first = -1;
    last = -1;

    /* Slashdot captchas have 7 characters */
    result = malloc(8 * sizeof(char));
    strcpy(result, "       ");

    /* Clean image a bit */
    tmp1 = filter_detect_lines(img);
    tmp2 = filter_fill_holes(tmp1);

    /* Detect small objects to guess image orientation */
    tmp3 = filter_median(tmp2);
    tmp4 = filter_equalize(tmp3, 200);
    count_objects(tmp4);

    /* Invert rotation and find glyphs */
    tmp5 = rotate(tmp2);
    tmp6 = filter_median(tmp5);
    tmp7 = find_glyphs(tmp6);

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

static struct image *count_objects(struct image *img)
{
    struct image *dst;
    int gotblack = 1;
    int x, y, i;
    int r, g, b;

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    while(gotblack)
    {
        gotblack = 0;
        for(y = 0; y < dst->height; y++)
            for(x = 0; x < dst->width; x++)
            {
                getpixel(dst, x, y, &r, &g, &b);
                if(r == 0 && g == 0 && b == 0)
                {
                    gotblack = 1;
                    filter_flood_fill(dst, x, y, 254 - objects, 0, 0);
                    objects++;
                }
            }
    }

    //printf("%i objects\n", objects);

    for(i = 0; i < objects; i++)
    {
        objlist[i].ymin = dst->height;
        objlist[i].ymax = 0;

        for(y = 0; y < dst->height; y++)
            for(x = 0; x < dst->width; x++)
            {
                getpixel(dst, x, y, &r, &g, &b);
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
            filter_flood_fill(dst, objlist[i].xmin, objlist[i].ymin, 0, 0, 255);
        }
    }

#if 0
    { CvPoint A, B;
      A.x = (objlist[first].xmin + objlist[first].xmax) / 2;
      A.y = (objlist[first].ymin + objlist[first].ymax) / 2;
      B.x = (objlist[last].xmin + objlist[last].xmax) / 2;
      B.y = (objlist[last].ymin + objlist[last].ymax) / 2;
      cvLine(dst, A, B, 0, 2.0, 0);
    }
#endif

    return dst;
}

static struct image *rotate(struct image *img)
{
    struct image *dst;
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

    dst = image_new(img->width * FACTOR, img->height * FACTOR);

    for(y = 0; y < img->height * FACTOR; y++)
        for(x = 0; x < img->width * FACTOR; x++)
        {
            xtmp = 1.0 * (x - img->width * FACTOR / 2) / FACTOR;
            ytmp = 1.0 * (y - img->height * FACTOR / 2) / FACTOR;
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
            setpixel(dst, x, y, r, g, b);
        }

    return dst;
}

static struct image *cut_cells(struct image *img)
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

    for(x = 0; x < img->width; x++)
    {
        setpixel(dst, x, 0, 255, 255, 255);
        setpixel(dst, x, img->height - 1, 255, 255, 255);
    }

    for(y = 0; y < img->height; y++)
        for(x = 0; x < 7; x++)
        {
            setpixel(dst, x * img->width / 7, y, 255, 255, 255);
            setpixel(dst, (x + 1) * img->width / 7 - 1, y, 255, 255, 255);
        }

    return dst;
}

static struct image *find_glyphs(struct image *img)
{
    char all[] = "abcdefgijkmnpqrstvwxyz";
    struct
    {
        int xmin, xmax, ymin, ymax;
        int count;
    }
    glyphs[22];
    struct image *dst;
    struct image *font = image_load(FONTNAME);
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, incell = 0, count = 0, startx = 0, cur = 0;
    int distmin, distx, disty, distch;

    if(!font)
    {
        fprintf(stderr, "cannot load font %s\n", FONTNAME);
        exit(-1);
    }

    dst = image_new(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, 255, g, 255);
        }

    for(x = 0; x < font->width; x++)
    {
        int found = 0;
        for(y = 0; y < font->height; y++)
        {
            getpixel(font, x, y, &r, &g, &b);
            if(r < 128)
            {
                found = 1;
                count += (255 - r);
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
            xmax = x;
#if 0
            ymin = font->height;
            ymax = 0;
            for(y = 0; y < font->height; y++)
            {
                int newx;
                int gotit = 0;
                for(newx = xmin; newx < xmax; newx++)
                {
                    getpixel(font, newx, y, &r, &g, &b);
                    if(r < 128)
                    {
                        gotit = 1;
                        break;
                    }
                }
                if(gotit)
                {
                    if(ymin > y) ymin = y;
                    if(ymax <= y) ymax = y + 1;
                }
            }
#else
            ymin = 0;
            ymax = font->height;
#endif
            glyphs[i].xmin = xmin;
            glyphs[i].xmax = xmax;
            glyphs[i].ymin = ymin;
            glyphs[i].ymax = ymax;
            glyphs[i].count = count;
            count = 0;
            i++;
        }
    }

    if(i != 22)
    {
        printf("error: could not find 22 glyphs in font\n");
        exit(-1);
    }

    while(cur < 7)
    {
        /* Try to find 1st letter */
        distmin = 999999999;
        for(i = 0; i < 22; i++)
        {
            int localmin = 99999999, localx, localy;
//if(all[i] == 'i') continue;
            xmin = glyphs[i].xmin;
            ymin = glyphs[i].ymin;
            xmax = glyphs[i].xmax;
            ymax = glyphs[i].ymax;
            //printf("trying to find %c (%i×%i) - ", all[i], xmax - xmin, ymax - ymin);
            for(y = -5 * FACTOR; y < 5 * FACTOR; y++)
                for(x = startx - 5 * FACTOR; x < startx + 5 * FACTOR; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font, xmin + z, ymin + t, &r);
                            getgray(img, x + z, y + t, &r2);
                            dist += abs(r - r2);
                        }
    //                printf("%i %i -> %i\n", x, y, dist);
                    //dist /= sqrt(xmax - xmin);
                    dist = dist * 128 / glyphs[i].count;
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

        //fprintf(stderr, "%i (%i,%i)\n", distmin, distx - startx, disty);
        //printf("min diff: %c - %i (%i, %i)\n", all[distch], distmin, distx, disty);

        /* Print min glyph */
        xmin = glyphs[distch].xmin;
        ymin = glyphs[distch].ymin;
        xmax = glyphs[distch].xmax;
        ymax = glyphs[distch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                getpixel(font, xmin + x, ymin + y, &r, &g, &b);
                if(r > 128) continue;
                setpixel(dst, distx + x, disty + y, r, g, b);
            }

        startx = distx + xmax - xmin;
        result[cur++] = all[distch];
    }

    return dst;
}

