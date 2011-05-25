/*
 * image.c: image I/O functions
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

#include "config.h"
#include "common.h"

#if defined(HAVE_SDL_IMAGE_H)
#   include <SDL_image.h>
#elif defined(HAVE_IMLIB2_H)
#   include <Imlib2.h>
#elif defined(HAVE_CV_H)
#   include <cv.h>
#   include <highgui.h>
#else
#   error "No imaging library"
#endif

struct image *image_load(const char *name)
{
    struct image *img;
#if defined(HAVE_SDL_IMAGE_H)
    SDL_Surface *priv = IMG_Load(name);
#elif defined(HAVE_IMLIB2_H)
    Imlib_Image priv = imlib_load_image(name);
#elif defined(HAVE_CV_H)
    IplImage *priv = cvLoadImage(name, -1);
#endif

    if(!priv)
        return NULL;

#if defined(HAVE_SDL_IMAGE_H)
    if(priv->format->BytesPerPixel == 1)
    {
        img = image_new(priv->w, priv->h);
        SDL_BlitSurface(priv, NULL, img->priv, NULL);
        SDL_FreeSurface(priv);
        return img;
    }
#endif

    img = (struct image *)malloc(sizeof(struct image));
#if defined(HAVE_SDL_IMAGE_H)
    img->width = priv->w;
    img->height = priv->h;
    img->pitch = priv->pitch;
    img->channels = priv->format->BytesPerPixel;
    img->pixels = priv->pixels;
#elif defined(HAVE_IMLIB2_H)
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

struct image *image_new(int width, int height)
{
    struct image *img;
#if defined(HAVE_SDL_IMAGE_H)
    SDL_Surface *priv;
    Uint32 rmask, gmask, bmask, amask;
#   if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x00000000;
#   else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0x00000000;
#   endif
    priv = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                rmask, gmask, bmask, amask);
#elif defined(HAVE_IMLIB2_H)
    Imlib_Image priv = imlib_create_image(width, height);
#elif defined(HAVE_CV_H)
    IplImage *priv = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
#endif

    if(!priv)
        return NULL;

    img = (struct image *)malloc(sizeof(struct image));
#if defined(HAVE_SDL_IMAGE_H)
    img->width = priv->w;
    img->height = priv->h;
    img->pitch = priv->pitch;
    img->channels = priv->format->BytesPerPixel;
    img->pixels = priv->pixels;
#elif defined(HAVE_IMLIB2_H)
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

struct image *image_dup(struct image *img)
{
    struct image *dst;
    int x, y;
    dst = image_new(img->width, img->height);
    for(y = 0; y < img->height; y++)
    {
        for(x = 0; x < img->width; x++)
        {
            int r, g, b;
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }
    }
    return dst;
}

void image_free(struct image *img)
{
#if defined(HAVE_SDL_IMAGE_H)
    SDL_FreeSurface(img->priv);
#elif defined(HAVE_IMLIB2_H)
    imlib_context_set_image(img->priv);
    imlib_free_image();
#elif defined(HAVE_CV_H)
    IplImage *iplimg;
    iplimg = (IplImage *)img->priv;
    cvReleaseImage(&iplimg);
#endif

    free(img);
}

void image_save(struct image *img, const char *name)
{
#if defined(HAVE_SDL_IMAGE_H)
    SDL_SaveBMP(img->priv, name);
#elif defined(HAVE_IMLIB2_H)
    imlib_context_set_image(img->priv);
    imlib_save_image(name);
#elif defined(HAVE_CV_H)
    cvSaveImage(name, img->priv, 0);
#endif
}

void image_swap(struct image *img1, struct image *img2)
{
    struct image tmp;
    memcpy(&tmp, img1, sizeof(tmp));
    memcpy(img1, img2, sizeof(tmp));
    memcpy(img2, &tmp, sizeof(tmp));
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

