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
#include <getopt.h>

#include "config.h"
#include "common.h"

int main(int argc, char *argv[])
{
    char *mode = "auto";

    int c;
    int digit_optind = 0;

    for(;;)
    {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] =
        {
            { "mode", 1, 0, 'm' },
            { "help", 0, 0, 'h' },
            { "version", 0, 0, 'v' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "hm:v", long_options, &option_index);
        if(c == -1)
            break;

        switch(c)
        {
        case 'h': /* --help */
            printf("Usage: %s [OPTION]... FILE...\n", argv[0]);
            printf("  -m, --mode      force operating mode\n");
            printf("  -h, --help      display this help and exit\n");
            printf("  -v, --version   output version information and exit\n");
            return 0;
        case 'm': /* --mode */
            mode = optarg;
            break;
        case 'v': /* --version */
            printf("pwntcha (CAPTCHA decoder) %s\n", VERSION);
            printf("Written by Sam Hocevar.\n");
            printf("\n");
            printf("Copyright (C) 2004-2005 Sam Hocevar <sam@zoy.org>\n");
            printf("This is free software; see the source for copying conditions.  There is NO\n");
            printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        case '?':
            break;
        default:
            printf("%s: invalid option -- %i\n", argv[0], c);
            printf("Try `%s --help' for more information.\n", argv[0]);
            return 1;
        }
    }

    if(optind >= argc)
    {
        printf("%s: too few arguments\n", argv[0]);
        printf("Try `%s --help' for more information.\n", argv[0]);
        return 1;
    }

    for(; optind < argc; optind++)
    {
        char *input = argv[optind], *result;
        struct image *img;

        img = image_load(argv[optind]);
        if(!img)
        {
            fprintf(stderr, "%s: cannot load %s\n", argv[0], input);
            printf("\n");
            continue;
        }

        result = decode_slashdot(img);
        if(!result)
        {
            fprintf(stderr, "%s: sorry, decoding failed\n", argv[0]);
            printf("\n");
            continue;
        }

        printf("%s\n", result);
        free(result);
    }

    return 0;
}

