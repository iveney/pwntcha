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

/* debug function */
void dprintf(const char *fmt, ...);

/* available CAPTCHA decoders */
char *decode_phpbb(struct image *img);
char *decode_scode(struct image *img);
char *decode_slashdot(struct image *img);
char *decode_test(struct image *img);

/* image operations */
struct image *image_load(char *name);
struct image *image_new(int width, int height);
void image_free(struct image *img);
void image_display(struct image *img);
int getgray(struct image *img, int x, int y, int *g);
int getpixel(struct image *img, int x, int y, int *r, int *g, int *b);
int setpixel(struct image *img, int x, int y, int r, int g, int b);

/* image filters */
void filter_flood_fill(struct image *img, int x, int y, int r, int g, int b);
struct image *filter_fill_holes(struct image *img);
struct image *filter_dup(struct image *img);
struct image *filter_detect_lines(struct image *img);
struct image *filter_equalize(struct image *img, int threshold);
struct image *filter_trick(struct image *img);
struct image *filter_smooth(struct image *img);
struct image *filter_median(struct image *img);
struct image *filter_contrast(struct image *img);

