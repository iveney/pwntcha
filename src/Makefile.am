
SUBDIRS = authimage clubic htmlformguide java linuxfr livejournal lmt movabletype \
          paypal phpbb scode slashdot ticketmaster tickets vbulletin xanga

bin_PROGRAMS = pwntcha
pwntcha_SOURCES = \
    main.c \
    filter.c \
    font.c \
    image.c \
    common.h \
    easter-eggs.c \
    test.c \
    $(NULL)
pwntcha_CFLAGS = $(imaging_cflags) -Wall -O6
pwntcha_LDFLAGS = $(imaging_ldflags)
pwntcha_LDADD = $(SUBDIRS:%=%/libdecoder.a)

if USE_SDL
imaging_cflags = `sdl-config --cflags`
imaging_ldflags = `sdl-config --libs` -lSDL_image
else
if USE_IMLIB2
imaging_cflags = `imlib2-config --cflags` -DX_DISPLAY_MISSING=1
imaging_ldflags = `imlib2-config --libs`
else
if USE_OPENCV
imaging_cflags = `pkg-config opencv --cflags`
#imaging_ldflags = `pkg-config opencv --libs opencv highgui`
imaging_ldflags = `pkg-config opencv --libs`
else
imaging_cflags =
imaging_ldflags =
endif
endif
endif

