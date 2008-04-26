/*
 * test.c: test captchas
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
#include <limits.h>
#include <math.h>

#include "config.h"
#include "common.h"

/* Main function */
char *decode_test(struct image *img)
{
    char *result;
    struct image *tmp;

    /* phpBB captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    result[0] = 0;

    tmp = image_dup(img);
    filter_smooth(tmp);
    filter_median(tmp);
    filter_threshold(tmp, 130);
    filter_median(tmp);

    image_free(tmp);

    return result;
}

/* The following functions are local */

