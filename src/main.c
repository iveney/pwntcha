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
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "common.h"

int main(int argc, char *argv[])
{
    char *result;

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <image>\n", argv[0]);
        return -1;
    }

    result = slashdot_decode(argv[1]);
    if(!result)
        return -1;

    printf("%s\n", result);

    return 0;
}

