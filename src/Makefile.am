NULL = 

bin_PROGRAMS = pwntcha
pwntcha_CFLAGS = $(ADDITIONAL_CFLAGS) -Wall -O6
pwntcha_LDFLAGS = $(ADDITIONAL_LDFLAGS)
pwntcha_SOURCES = main.c image.c filters.c slashdot.c common.h

if USE_IMLIB2
ADDITIONAL_CFLAGS = `imlib2-config --cflags` -DX_DISPLAY_MISSING=1
ADDITIONAL_LDFLAGS = `imlib2-config --libs`
else
if USE_OPENCV
ADDITIONAL_CFLAGS = `opencv-config --cflags`
ADDITIONAL_LDFLAGS = `opencv-config --libs opencv highgui`
else
ADDITIONAL_CFLAGS =
ADDITIONAL_LDFLAGS =
endif
endif

