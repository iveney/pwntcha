/*
 * main.c: main function
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

int main(int argc, char *argv[])
{
    struct image *img;
    char *result;

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <image>\n", argv[0]);
        return -1;
    }

    img = image_load(argv[1]);
    if(!img)
    {
        fprintf(stderr, "cannot load %s\n", argv[1]);
        return -1;
    }

    result = decode_slashdot(img);
    if(!result)
    {
        fprintf(stderr, "sorry, decoding failed\n");
        return -1;
    }

    printf("%s\n", result);

    return 0;
}

