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

#if defined(WIN32)
#   include <windows.h>
#   include <ocidl.h>
#   include <olectl.h>
BOOL oleload(LPCTSTR name, LPPICTURE* pic);
struct priv
{
    HBITMAP bitmap;
    BITMAPINFO info;
};
#elif defined(HAVE_SDL_IMAGE_H)
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
#if defined(WIN32)
    struct priv *priv = malloc(sizeof(struct priv));
    LPPICTURE pic = NULL;
    HDC dc;
    long scrwidth = 0, scrheight = 0;
    int width, height;
    void *data = NULL;
#elif defined(HAVE_SDL_IMAGE_H)
    SDL_Surface *priv = IMG_Load(name);
#elif defined(HAVE_IMLIB2_H)
    Imlib_Image priv = imlib_load_image(name);
#elif defined(HAVE_CV_H)
    IplImage *priv = cvLoadImage(name, -1);
#endif

    if(!priv)
        return NULL;

#if defined(WIN32)
    if(!oleload((LPCTSTR)name, &pic))
    {
        free(priv);
        return NULL;
    }

#if 0
    for(i = 0; ; i++) 
    {
        DEVMODE devMode;
        devMode.dmSize = sizeof(DEVMODE);

        if(EnumDisplaySettings(NULL, i, &devMode) != 1)
            break;

        printf("mode %i x %i - %i\n", (int)devMode.dmPelsWidth,
               (int)devMode.dmPelsHeight, (int)devMode.dmBitsPerPel);
    }
#endif

    pic->lpVtbl->get_CurDC(pic, &dc);

    if(GetDeviceCaps(dc, BITSPIXEL) < 24)
    {
        fprintf(stderr, "%s: 24bpp screen depth or better required\n", argv0);
        DeleteDC(dc);
        free(priv);
        return NULL;
    }

    pic->lpVtbl->get_Width(pic, &scrwidth);
    pic->lpVtbl->get_Height(pic, &scrheight);
    width = (scrwidth * GetDeviceCaps(dc, LOGPIXELSX) + 2540 / 2) / 2540;
    height = (scrheight * GetDeviceCaps(dc, LOGPIXELSY) + 2540 / 2) / 2540;

    memset(&priv->info, 0, sizeof(BITMAPINFO));
    priv->info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    priv->info.bmiHeader.biBitCount = 24;
    priv->info.bmiHeader.biWidth = width;
    priv->info.bmiHeader.biHeight = -height;
    priv->info.bmiHeader.biCompression = BI_RGB;
    priv->info.bmiHeader.biPlanes = 1;

    priv->bitmap = CreateDIBSection(dc, &priv->info, DIB_RGB_COLORS, &data, 0, 0);
    SelectObject(dc, priv->bitmap);
    pic->lpVtbl->Render(pic, dc, 0, 0, width, height,
                        0, scrheight, scrwidth, -scrheight, NULL);
    pic->lpVtbl->Release(pic);
    DeleteDC(dc);
#elif defined(HAVE_SDL_IMAGE_H)
    if(priv->format->BytesPerPixel == 1)
    {
        img = image_new(priv->w, priv->h);
        SDL_BlitSurface(priv, NULL, img->priv, NULL);
        SDL_FreeSurface(priv);
        return img;
    }
#endif

    img = (struct image *)malloc(sizeof(struct image));
#if defined(WIN32)
    img->width = width;
    img->height = height;
    img->pitch = 3 * width;
    img->channels = 3;
    img->pixels = data;
#elif defined(HAVE_SDL_IMAGE_H)
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
#if defined(WIN32)
    struct priv *priv = malloc(sizeof(struct priv));
    HDC dc;
    void *data = NULL;
#elif defined(HAVE_SDL_IMAGE_H)
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
#if defined(WIN32)
    dc = CreateCompatibleDC(NULL);
    if(GetDeviceCaps(dc, BITSPIXEL) < 24)
    {
        fprintf(stderr, "a screen depth of at least 24bpp is required\n");
        DeleteDC(dc);
        free(priv);
        return NULL;
    }
    priv->info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    priv->info.bmiHeader.biWidth = width;
    priv->info.bmiHeader.biHeight = -height;
    priv->info.bmiHeader.biCompression = BI_RGB;
    priv->info.bmiHeader.biBitCount = 24;
    priv->info.bmiHeader.biPlanes = 1;
    priv->bitmap = CreateDIBSection(dc, &priv->info,
                                    DIB_RGB_COLORS, &data, 0, 0);
    DeleteDC(dc);
    img->width = width;
    img->height = height;
    img->pitch = 3 * width;
    img->channels = 3;
    img->pixels = data;
#elif defined(HAVE_SDL_IMAGE_H)
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

void image_free(struct image *img)
{
#if defined(WIN32)
    struct priv *priv = img->priv;
    DeleteObject(priv->bitmap);
    free(img->priv);
#elif defined(HAVE_SDL_IMAGE_H)
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
#if defined(WIN32)

#elif defined(HAVE_SDL_IMAGE_H)
    SDL_SaveBMP(img->priv, name);
#elif defined(HAVE_IMLIB2_H)
    imlib_context_set_image(img->priv);
    imlib_save_image(name);
#elif defined(HAVE_CV_H)
    cvSaveImage(name, img->priv);
#endif
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

#if defined(WIN32)
BOOL oleload(LPCTSTR name, LPPICTURE* pic)
{
    HRESULT ret;
    HANDLE h;
    DWORD size, read = 0;
    LPVOID data;
    HGLOBAL buffer;
    LPSTREAM stream = NULL;

    h = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return FALSE;

    size = GetFileSize(h, NULL);
    if(size == (DWORD)-1)
    {
        CloseHandle(h);
        return FALSE;
    }

    buffer = GlobalAlloc(GMEM_MOVEABLE, size);
    if(!buffer)
    {
        CloseHandle(h);
        return FALSE;
    }

    data = GlobalLock(buffer);
    if(!data)
    {
        GlobalUnlock(buffer);
        CloseHandle(h);
        return FALSE;
    }

    ret = ReadFile(h, data, size, &read, NULL);
    GlobalUnlock(buffer);
    CloseHandle(h);

    if(!ret)
        return FALSE;

    ret = CreateStreamOnHGlobal(buffer, TRUE, &stream);
    if(!SUCCEEDED(ret))
    {
        if(stream)
            stream->lpVtbl->Release(stream);
        return FALSE;
    }

    if(!stream)
        return FALSE;

    if(*pic)
        (*pic)->lpVtbl->Release(*pic);

    ret = OleLoadPicture(stream, size, FALSE, &IID_IPicture, (PVOID *)pic);
    stream->lpVtbl->Release(stream);

    if(!SUCCEEDED(ret))
        return FALSE;

    if(!*pic)
        return FALSE;

    return TRUE;
}
#endif

