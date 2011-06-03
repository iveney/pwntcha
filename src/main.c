/*
 * main.c: main function
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
#include <getopt.h>
#include <stdarg.h>

#include "config.h"
#include "common.h"

#ifdef HAVE_GETOPT_LONG
#   define MOREINFO "Try `%s --help' for more information.\n"
#else
#   define MOREINFO "Try `%s -h' for more information.\n"
#endif

/* Used for the debug messages */
char *argv0 = NULL;
char *share = NULL;
int debug = 1;

int main(int argc, char *argv[])
{
    char *mode = "auto";
    int c;
    int digit_optind = 0;

    argv0 = argv[0];
    share = "share";

    for(;;)
    {
        int this_option_optind = optind ? optind : 1;
#ifdef HAVE_GETOPT_LONG
        int option_index = 0;
        static struct option long_options[] =
        {
            { "help", 0, 0, 'h' },
            { "mode", 1, 0, 'm' },
            { "share", 1, 0, 's' },
            { "quiet", 0, 0, 'q' },
            { "version", 0, 0, 'v' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "hm:s:qv", long_options, &option_index);
#else
        c = getopt(argc, argv, "hm:s:qv");
#endif
        if(c == -1)
            break;

        switch(c)
        {
        case 'h': /* --help */
            printf("Usage: %s [OPTION]... IMAGE...\n", argv[0]);
#ifdef HAVE_GETOPT_LONG
            printf("  -m, --mode <mode>  force operating mode\n");
            printf("  -s, --share <dir>  specify shared dir\n");
            printf("  -q, --quiet        do not print information messages\n");
            printf("  -h, --help         display this help and exit\n");
            printf("  -v, --version      output version information and exit\n");
#else
            printf("  -m <mode>   force operating mode\n");
            printf("  -s <dir>    specify shared dir\n");
            printf("  -q          do not print information messages\n");
            printf("  -h          display this help and exit\n");
            printf("  -v          output version information and exit\n");
#endif
            return 0;
        case 'm': /* --mode */
            mode = optarg;
            break;
        case 'q': /* --quiet */
            debug = 0;
            break;
        case 's': /* --quiet */
            share = optarg;
            break;
        case 'v': /* --version */
            printf("pwntcha (captcha decoder) %s\n", PACKAGE_VERSION);
            printf("Written by Sam Hocevar.\n");
            printf("\n");
            printf("Copyright (C) 2004-2005 Sam Hocevar <sam@zoy.org>\n");
            printf("This is free software; see the source for copying conditions.  There is NO\n");
            printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        case '?':
            printf(MOREINFO, argv[0]);
            return 1;
        default:
            printf("%s: invalid option -- %i\n", argv[0], c);
            printf(MOREINFO, argv[0]);
            return 1;
        }
    }

    if(optind >= argc)
    {
        printf("%s: too few arguments\n", argv[0]);
        printf(MOREINFO, argv[0]);
        return 1;
    }

    for(; optind < argc; optind++)
    {
        char *input = argv[optind], *result;
        struct image *img;
        int count;

        img = image_load(argv[optind]);
        if(!img)
        {
            pwnprint("cannot load %s\n", input);
            printf("\n");
            continue;
        }

        count = filter_count(img);
        pwnprint("image size %ix%i, %i colours\n",
                 img->width, img->height, count);

        if(!strcmp(mode, "test"))
            result = decode_test(img);
        else if(!strcmp(mode, "authimage"))
            result = decode_authimage(img);
        else if(!strcmp(mode, "clubic"))
            result = decode_clubic(img);
        else if(!strcmp(mode, "htmlformguide"))
            result = decode_htmlformguide(img);
        else if(!strcmp(mode, "java"))
            result = decode_java(img);
        else if(!strcmp(mode, "linuxfr"))
            result = decode_linuxfr(img);
        else if(!strcmp(mode, "livejournal"))
            result = decode_livejournal(img);
        else if(!strcmp(mode, "lmt"))
            result = decode_lmt(img);
        else if(!strcmp(mode, "paypal"))
            result = decode_paypal(img);
        else if(!strcmp(mode, "phpbb"))
            result = decode_phpbb(img);
        else if(!strcmp(mode, "scode"))
            result = decode_scode(img);
        else if(!strcmp(mode, "slashdot"))
            result = decode_slashdot(img);
        else if(!strcmp(mode, "vbulletin"))
            result = decode_vbulletin(img);
        else if(!strcmp(mode, "xanga"))
            result = decode_xanga(img);
        else
        {
            if(img->width == 155 && img->height == 50)
            {
                pwnprint("probably an authimage captcha\n");
                result = decode_authimage(img);
            }
            else if(img->width == 120 && img->height == 40)
            {
                pwnprint("probably a htmlformguide captcha\n");
                result = decode_htmlformguide(img);
            }
            else if(img->width == 175 && img->height == 35)
            {
                pwnprint("probably a livejournal captcha\n");
                result = decode_livejournal(img);
            }
            else if(img->width == 100 && img->height == 40 && count < 6)
            {
                pwnprint("probably a linuxfr captcha\n");
                result = decode_linuxfr(img);
            }
            else if(img->width == 69 && img->height == 35)
            {
                pwnprint("probably a lmt.lv captcha\n");
                result = decode_lmt(img);
            }
            else if(img->width == 208 && img->height == 26)
            {
                pwnprint("probably a Paypal captcha\n");
                result = decode_paypal(img);
            }
            else if(img->width == 320 && img->height == 50)
            {
                pwnprint("probably a phpBB captcha\n");
                result = decode_phpbb(img);
            }
            else if(img->width == 170 && img->height == 50)
            {
                pwnprint("probably a Xanga captcha\n");
                result = decode_xanga(img);
            }
            else if(img->height <= 40 && count < 10)
            {
                pwnprint("probably a scode/trencaspammers captcha\n");
                result = decode_scode(img);
            }
            else if(img->height <= 30 && count < 100)
            {
                pwnprint("probably a clubic captcha\n");
                result = decode_clubic(img);
            }
            else if(img->height == 69)
            {
                pwnprint("probably a slashdot captcha\n");
                result = decode_slashdot(img);
            }
            else if(img->width == 200 && img->height == 100)
            {
                pwnprint("probably a java captcha\n");
                result = decode_java(img);
            }
            else if(img->width == 290 && img->height == 80)
            {
                pwnprint("probably a ticketmaster captcha\n");
                result = decode_ticketmaster(img);
            }
            else if(img->width == 200 && img->height == 40)
            {
                pwnprint("probably a tickets.com captcha\n");
                result = decode_tickets(img);
            }
            else if(img->height == 61)
            {
                pwnprint("probably a vbulletin captcha\n");
                result = decode_vbulletin(img);
            }
            else if(img->width == 480 && img->height == 360 && count == 253)
            {
                pwnprint("probably not a captcha\n");
                result = decode_easter_eggs(img);
            }
            else
            {
                pwnprint("could not guess captcha type\n");
                printf("\n");
                image_free(img);
                continue;
            }
        }

        image_free(img);

        if(!result)
        {
            pwnprint("sorry, decoding failed\n");
            printf("\n");
            continue;
        }

        printf("%s\n", result);
        fflush(stdout);
        free(result);
    }

    return 0;
}

void pwnprint(const char *fmt, ...)
{
    va_list args;

    if(!debug)
        return;

    va_start(args, fmt);
    fprintf(stderr, "%s: ", argv0);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

