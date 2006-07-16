NULL = 

bin_PROGRAMS = pwntcha
pwntcha_SOURCES = \
    main.c \
    filter.c \
    font.c \
    image.c \
    common.h \
    authimage.c \
    clubic.c \
    java.c \
    linuxfr.c \
    livejournal.c \
    lmt.c \
    paypal.c \
    phpbb.c \
    scode.c \
    slashdot.c \
    tickets.c \
    ticketmaster.c \
    vbulletin.c \
    xanga.c \
    easter-eggs.c \
    test.c
pwntcha_CFLAGS = $(ADDITIONAL_CFLAGS) -Wall -O6
pwntcha_LDFLAGS = $(ADDITIONAL_LDFLAGS)
pwntcha_LDADD = $(ADDITIONAL_LDADD)

if USE_SDL
ADDITIONAL_CFLAGS = `sdl-config --cflags`
ADDITIONAL_LDFLAGS = `sdl-config --libs` -lSDL_image
ADDITIONAL_LDADD =
else
if USE_IMLIB2
ADDITIONAL_CFLAGS = `imlib2-config --cflags` -DX_DISPLAY_MISSING=1
ADDITIONAL_LDFLAGS = `imlib2-config --libs`
ADDITIONAL_LDADD =
else
if USE_OPENCV
ADDITIONAL_CFLAGS = `opencv-config --cflags`
ADDITIONAL_LDFLAGS = `opencv-config --libs opencv highgui`
ADDITIONAL_LDADD =
else
ADDITIONAL_CFLAGS =
ADDITIONAL_LDFLAGS =
ADDITIONAL_LDADD =
endif
endif
endif

