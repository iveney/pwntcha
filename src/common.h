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

/* font structure */
struct font
{
    struct image *img;
    struct glyph
    {
        int xmin, xmax, ymin, ymax;
        int count; /* Black pixel count */
    } *glyphs;
};

/* global variables */
extern char *argv0;
extern char *share;

/* debug function */
void dprintf(const char *fmt, ...);

/* available CAPTCHA decoders */
char *decode_authimage(struct image *img);
char *decode_clubic(struct image *img);
char *decode_linuxfr(struct image *img);
char *decode_phpbb(struct image *img);
char *decode_scode(struct image *img);
char *decode_slashdot(struct image *img);
char *decode_vbulletin(struct image *img);
char *decode_xanga(struct image *img);
char *decode_test(struct image *img);

/* image operations */
struct image *image_load(const char *name);
struct image *image_new(int width, int height);
struct image *image_dup(struct image *img);
void image_free(struct image *img);
void image_save(struct image *img, const char *name);
void image_swap(struct image *img1, struct image *img2);
int getgray(struct image *img, int x, int y, int *g);
int getpixel(struct image *img, int x, int y, int *r, int *g, int *b);
int setpixel(struct image *img, int x, int y, int r, int g, int b);

/* image filters */
void filter_flood_fill(struct image *img, int x, int y, int r, int g, int b);
void filter_fill_holes(struct image *img);
void filter_scale(struct image *img, float ratio);
void filter_black_stuff(struct image *img);
void filter_detect_lines(struct image *img);
void filter_equalize(struct image *img, int threshold);
void filter_trick(struct image *img);
void filter_smooth(struct image *img);
void filter_median(struct image *img);
void filter_contrast(struct image *img);
void filter_crop(struct image *img, int xmin, int ymin, int xmax, int ymax);
int filter_count(struct image *img);

