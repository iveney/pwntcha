/*
 * image.c: image I/O functions
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

#include "config.h"
#include "common.h"

#if defined(HAVE_IMLIB2_H)
#   include <Imlib2.h>
#elif defined(HAVE_CV_H)
#   include <cv.h>
#   include <highgui.h>
#else
#   error "No imaging library"
#endif

struct image * load_image(char *name)
{
    struct image * img;
#if defined(HAVE_IMLIB2_H)
    Imlib_Image priv = imlib_load_image(name);
#elif defined(HAVE_CV_H)
    IplImage * priv = cvLoadImage(name, -1);
#endif

    if(!priv)
        return NULL;

    img = malloc(sizeof(struct image));
#if defined(HAVE_IMLIB2_H)
    imlib_context_set_image(priv);
    img->width = imlib_image_get_width();
    img->height = imlib_image_get_height();
    img->pitch = 4 * imlib_image_get_width();
    img->channels = 4;
    img->pixels = (char *)imlib_image_get_data();
#elif defined(HAVE_CV_H)
    img->width = priv->width;
    img->height = priv->height;
    img->pitch = priv->widthStep;
    img->channels = priv->nChannels;
    img->pixels = priv->imageData;
#endif
    img->priv = (void *)priv;

    return img;
}

struct image * new_image(int width, int height)
{
    struct image * img;
#if defined(HAVE_IMLIB2_H)
    Imlib_Image priv = imlib_create_image(width, height);
#elif defined(HAVE_CV_H)
    IplImage * priv = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
#endif

    if(!priv)
        return NULL;

    img = malloc(sizeof(struct image));
#if defined(HAVE_IMLIB2_H)
    imlib_context_set_image(priv);
    img->width = imlib_image_get_width();
    img->height = imlib_image_get_height();
    img->pitch = 4 * imlib_image_get_width();
    img->channels = 4;
    img->pixels = (char *)imlib_image_get_data();
#elif defined(HAVE_CV_H)
    img->width = priv->width;
    img->height = priv->height;
    img->pitch = priv->widthStep;
    img->channels = priv->nChannels;
    img->pixels = priv->imageData;
#endif
    img->priv = (void *)priv;

    return img;
}

int getgray(struct image *img, int x, int y, int *g)
{
    if(x < 0 || y < 0 || x >= img->width || y >= img->height)
    {
        *g = 255;
        return -1;
    }

    *g = (unsigned char)img->pixels[y * img->pitch + x * img->channels + 1];

    return 0;
}

int getpixel(struct image *img, int x, int y, int *r, int *g, int *b)
{
    if(x < 0 || y < 0 || x >= img->width || y >= img->height)
    {
        *r = 255;
        *g = 255;
        *b = 255;
        return -1;
    }

    *b = (unsigned char)img->pixels[y * img->pitch + x * img->channels];
    *g = (unsigned char)img->pixels[y * img->pitch + x * img->channels + 1];
    *r = (unsigned char)img->pixels[y * img->pitch + x * img->channels + 2];

    return 0;
}

int setpixel(struct image *img, int x, int y, int r, int g, int b)
{
    if(x < 0 || y < 0 || x >= img->width || y >= img->height)
        return -1;

    img->pixels[y * img->pitch + x * img->channels] = b;
    img->pixels[y * img->pitch + x * img->channels + 1] = g;
    img->pixels[y * img->pitch + x * img->channels + 2] = r;

    return 0;
}

void display_image(struct image *img)
{
#if defined(HAVE_IMLIB2_H)
    //char name[BUFSIZ];
    //static int i = 0;
    //sprintf(name, "image%i-%ix%i.png", i++, img->width, img->height);
    //imlib_context_set_image(img->priv);
    //imlib_save_image(name);
    //fprintf(stderr, "saved to %s\n", name);
#elif defined(HAVE_CV_H)
    char name[BUFSIZ];
    sprintf(name, "Image %p (%i x %i)", img, img->width, img->height);
    cvNamedWindow(name, 0);
    cvShowImage(name, img->priv);
    cvResizeWindow(name, 320, 120);
#endif
}

