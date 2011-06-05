/*
 * paypal.c: decode Paypal captchas
 * $Id$
 *
 * Copyright: (c) 2005 Sam Hocevar <sam@zoy.org>
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

static void find_glyphs(struct image *img);
static int vertical_pixel_count(struct image *img, int x);
static int horizontal_pixel_count(struct image *img, int y);

/* Our macros */
char *result;

/* Main function */
char *decode_paypal(struct image *img)
{
    struct image *tmp;

    /* paypal captchas have 8 characters */
    result = malloc(9 * sizeof(char));
    strcpy(result, "        ");

    tmp = image_dup(img);

    /* apply greyscale filter */
    filter_greyscale(tmp);

    /* thresholding */
    filter_threshold(tmp, 30);

    /* further cleaning */
    int x, y, r, g, b;
    for(y = 0; y < img->height; y++)
    {
        /* check horizontally to get rid of (horizontal)lines with just a few pixel on */
        int count = horizontal_pixel_count(tmp, y);
        if ((count > 0) && (count < 6))
        {
            for(x = 0; x < tmp->width; x++)
            {
                setpixel(tmp, x, y, 255, 255, 255);
            }
        }

        /* get rid of some remaining noise pixels */
        for(x = 0; x < tmp->width; x++)
        {
            getpixel(tmp, x, y, &r, &g, &b);

            if ((x > 0) && (y > 0) && (x < tmp->width-1) && (y < tmp->height-1))
            {
                int ra, rb, rc, rd, re, rf, rg, rh;

                getpixel(tmp, x-1, y-1, &ra, &g, &b);
                getpixel(tmp, x, y-1, &rb, &g, &b);
                getpixel(tmp, x+1, y-1, &rc, &g, &b);
		getpixel(tmp, x-1, y, &rd, &g, &b);
		getpixel(tmp, x+1, y, &re, &g, &b);
                getpixel(tmp, x-1, y+1, &rf, &g, &b);
                getpixel(tmp, x, y+1, &rg, &g, &b);
                getpixel(tmp, x+1, y+1, &rh, &g, &b);

		if ((ra + rb + rc + rd + re + rf + rg + rh) >= 2040)
                {
                    setpixel(tmp, x, y, 255, 255, 255);
                }
            }
        }
    }

    //image_save(tmp, "output.bmp");

    find_glyphs(tmp);

    image_free(tmp);

    return result;
}

static void find_glyphs(struct image *img)
{
    #define TEMPLATE_GLYPHS_NUM 153
    int x = 0, y, z, r, g, b;
    int xmin = 0, xmax = 0, ymin = 0, ymax = 0;
    int cur = 0;
    char *tmplmap;
    char filename[64];

    /* read char template mapping file */
    sprintf(filename, "src/%s/templates/tmpl.map", DECODER);
    FILE *fh = fopen(filename, "rb");	
    if(!fh)
    {
        sprintf(filename, "%s/%s/templates/tmpl.map", share, DECODER);
        fh = fopen(filename, "r");
    }
    if(!fh)
    {
        fprintf(stderr, "%s: cannot load template map %s\n", argv0, filename);
        return;
    }

    tmplmap = malloc(1024);
    if (fread (tmplmap, 1, TEMPLATE_GLYPHS_NUM, fh) != TEMPLATE_GLYPHS_NUM) 
    {
        fprintf(stderr, "%s: cannot read template map %s\n", argv0, filename);
        return;
    }
    fclose(fh);

    /* locate min and max y */
    for(y = 0; y < img->height; y++)
    {
        if (horizontal_pixel_count(img, y) > 0)
        {
            ymin = y;
            break;
        }
    }
    for(y = img->height-1; y >= 0; y--)
    {
        if (horizontal_pixel_count(img, y) > 0)
        {
            ymax = y;
            break;
        }
    }

    /* proceed to segmentation */
    while ((cur < 8) && (x < img->width))
    {
        int count = vertical_pixel_count(img, x);

        if (count > 0)
        {
            for (xmax = x+11 ; xmax < img->width; xmax++)
            {
		if (vertical_pixel_count(img, xmax) == 0) 
                {
                    xmin = x;
                    x = xmax;
                    break;
                }
            }

            //printf("glyph at %d:%d to %d:%d\n", xmin, ymin, xmax, ymax);

            /* here a glyph is located, crop it */

            struct image *_tmp = image_dup(img);
            filter_crop(_tmp, xmin, ymin, xmax, ymax);

            /* saving tmp here lead to save a template glyph */
            //sprintf(filename, "glyph_%02d.bmp", cur);
            //image_save(_tmp, filename);

            struct image *tmp = image_new(20, 20);
            filter_flood_fill(tmp, 0, 0, 255, 255, 255);
            int c, d;
            for (d = 0 ; d < _tmp->height; d++)
            {
                for (c = 0 ; c < _tmp->width; c++)
                {
                    getpixel(_tmp, c, d, &r, &g, &b);
                    setpixel(tmp, c, d, r, g, b);
                }
            }	
            image_free(_tmp);

            int bestdist = INT_MAX;
            int besttmpl = 0;

            /* look in all templates */
            for (z = 0; z < TEMPLATE_GLYPHS_NUM; z++)
            {
                sprintf(filename, "src/%s/templates/tmpl_%03d.bmp", DECODER, z);
                struct image *_tmpl = image_load(filename);
                if(!_tmpl)
                {
                    sprintf(filename, "%s/%s/templates/tmpl_%03d.bmp", share, DECODER, z);
                    _tmpl = image_load(filename);
                }
                if(!_tmpl)
                {
                    fprintf(stderr, "%s: cannot load template %s\n", argv0, filename);
                    return;
                }
                struct image *tmpl = image_new(20, 20);
                filter_flood_fill(tmpl, 0, 0, 255, 255, 255);
                for (d = 0 ; d < _tmpl->height; d++)
                {
                    for (c = 0 ; c < _tmpl->width; c++)
                    {
                        getpixel(_tmpl, c, d, &r, &g, &b);
                        setpixel(tmpl, c, d, r, g, b);
                    }
                }	
                image_free(_tmpl);

                /* try to find the template that fits the best */
                int s, t, dist;
                dist = 0;
                for(t = 0; t < 20; t++)
                    for(s= 0; s < 20; s++)
                    {
                        int r2;
                        getgray(tmpl, s, t, &r);
                        getgray(tmp, s, t, &r2);
                        dist += abs(r - r2);
                    }

		if (dist < bestdist)
                {
                    bestdist = dist;
                    besttmpl = z;
                }

                image_free(tmpl);
            }

            //printf("bestdist=%d with tmpl_%03d.bmp char: %c\n", bestdist, besttmpl, tmplmap[besttmpl]);

            result[cur++] = tmplmap[besttmpl];
        }
        x++;
    }

    free(tmplmap);
}

static int vertical_pixel_count(struct image *img, int x)
{
    int y, r, g, b;
    int count = 0;

    for(y = 0; y < img->height; y++)
    {
        getpixel(img, x, y, &r, &g, &b);

        if ((r + g + b) == 0)
        {
            count++;
        }
    }

    return count;
}

static int horizontal_pixel_count(struct image *img, int y)
{
    int x, r, g, b;
    int count = 0;

    for(x = 0; x < img->width; x++)
    {
        getpixel(img, x, y, &r, &g, &b);

        if ((r + g + b) == 0)
        {
            count++;
        }
    }

    return count;
}

