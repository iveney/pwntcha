/*
 * ticketmaster.c: decode ticketmaster captchas
 * $Id$
 *
 * Copyright: (c) 2006 Sam Hocevar <sam@zoy.org>
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

#include "config.h"
#include "common.h"

#define WIDTH 290
#define HEIGHT 80

int grid_y[HEIGHT], grid_x[WIDTH];

int dx_top[WIDTH], dy_top[WIDTH];
int dx_bottom[WIDTH], dy_bottom[WIDTH];

int perturbation_color(struct image *img, int x, int y);
int real_color(struct image *img, int x, int y);

int detect_grid(struct image *img);
int detect_lines(struct image *img, int top);

static inline int fastgetgray(struct image *img, int x, int y)
{
    int r, g, b;
    getpixel(img, x, y, &r, &g, &b);
    return (r + g + b + 1) / 3;
}

/* Main function */
char *decode_ticketmaster(struct image *img)
{
    struct image *tmp;
    int x, y;

    if(detect_grid(img))
        pwnprint("image has vertical grid\n");

    detect_lines(img, 1);
    detect_lines(img, 0);

    tmp = image_dup(img);

    /* Remove perturbations */
    for(y = 1; y < HEIGHT - 1; y++)
    {
        for(x = 1; x < WIDTH - 1; x++)
        {
            int i, j, ok = 1;

            if(perturbation_color(img, x, y) != 0)
                continue;

            for(j = -1; j <= 1; j++)
            {
                for(i = -1; i <= 1; i++)
                {
                    if(i == 0 && j == 0)
                        continue;

                    if(real_color(img, x + i, y + j)
                         != perturbation_color(img, x + i, y + j))
                        goto lol;
                }
            }

            if(!ok)
                continue;

            setpixel(tmp, x, y, 255, 255, 255);

lol: continue;
        }
    }

    /* Lol, display lines */
    for(x = 1; x < WIDTH - 1; x++)
    {
        if(dy_top[x])
            for(y = 0; y < HEIGHT; y++)
            {
                int newx = (x * 1024 + dx_top[x] + (y * dy_top[x])) / 1024;
                setpixel(tmp, newx, y, 0, 255, 0);
            }

        if(dy_bottom[x])
            for(y = 0; y < HEIGHT; y++)
            {
                int newx = (x * 1024 + dx_bottom[x] + (y * dy_bottom[x])) / 1024;
                setpixel(tmp, newx, (HEIGHT - y - 1), 0, 255, 0);
            }
    }

image_save(tmp, "ticketmaster-output.bmp");
    image_free(tmp);

    return strdup("lol");
}

int real_color(struct image *img, int x, int y)
{
    return fastgetgray(img, x, y) < 40 ? 0 : 255;
}

int perturbation_color(struct image *img, int x, int y)
{
    if(grid_x[x]) return 0;

    if(grid_y[y]) return 0;

    return 255;
}

int detect_lines(struct image *img, int top)
{
    int x, y, i, j;

    int dx, dy;
    int *mydx, *mydy;

    if(top)
    {
        mydx = dx_top;
        mydy = dy_top;
    }
    else
    {
        mydx = dx_bottom;
        mydy = dy_bottom;
    }

    /* Detect top-bottom line candidates */
    for(x = 0; x < WIDTH; x++)
    {
        int candidate = 0, worstmissed;

        y = top ? 0 : HEIGHT - 3;
        for(j = 0; j < 3; j++)
        {
            for(i = -1; i <= 1; i++)
            {
                if(fastgetgray(img, x + i, y + j) < 40)
                {
                    candidate++;
                    break;
                }
            }
        }

        if(candidate < 3)
            continue;

        mydx[x] = mydy[x] = 0;

        worstmissed = 30;

        /* Refine slope, in 1/1024th of a pixel steps */
        for(dx = -1024; dx < 1024; dx += 128)
        {
            for(dy = 700; dy < 1300; dy += 16)
            {
                int missed = 0;

                for(y = 0; y < HEIGHT; y++)
                {
                    int newx = (x * 1024 + dx + (y * dy)) / 1024;
                    int gray = fastgetgray(img, newx, top ? y : (HEIGHT - y - 1));

                    if(gray > 40)
                    {
                        missed++;

                        if(missed >= worstmissed)
                            break;
                    }
                }

                if(missed < worstmissed)
                {
                    worstmissed = missed;
                    mydx[x] = dx;
                    mydy[x] = dy;
                }
            }
        }

        if(worstmissed == 30)
            continue;

        pwnprint("found a line at %i (+ %i), slope %i/1024\n", x, mydx[x], mydy[x]);
    }

    return 1;
}

int detect_grid(struct image *img)
{
    int x, y;

    int lines = 0, columns = 0;

    memset(grid_y, 0, sizeof(grid_y));
    memset(grid_x, 0, sizeof(grid_x));

    for(y = 0; y < HEIGHT; y++)
    {
        for(x = 0; x < WIDTH; x++)
        {
            if(fastgetgray(img, x, y) < 40)
            {
                grid_x[x]++;
                grid_y[y]++;
            }
        }
    }

    for(y = 0; y < HEIGHT; y++)
        if((grid_y[y] = (grid_y[y] > WIDTH * 90 / 100))) lines++;

    for(x = 0; x < WIDTH; x++)
        if((grid_x[x] = (grid_x[x] > HEIGHT * 90 / 100))) columns++;

    return lines * columns > 3 * 20;
}

