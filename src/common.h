/*
 * commin.h: common stuff
 * $Id$
 *
 * Copyright: (c) 2004 Sam Hocevar <sam@zoy.org>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License as published by Banlu Kemiyatorn. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

/* image structure */
struct image
{
    int width, height, pitch, channels;
    unsigned char *pixels;
    void *priv;
};

/* available CAPTCHA decoders */
char * slashdot_decode(char *image);

/* image operations */
struct image * load_image(char *name);
struct image * new_image(int width, int height);
int getgray(struct image *img, int x, int y, int *g);
int getpixel(struct image *img, int x, int y, int *r, int *g, int *b);
int setpixel(struct image *img, int x, int y, int r, int g, int b);

