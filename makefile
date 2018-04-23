# makefile for my1imgpro (basic image/vision processing library)
# - using libav & SDL for interfacing & display

TESTIMG = my1imgtest
OBJSIMG = my1imgpro.o my1imgutil.o my1imgmath.o my1imgfpo.o
OBJSIMG += my1imgbmp.o my1imgpnm.o my1imgfilt.o my1imgtest.o
TESTVIS = my1vistest
OBJSVIS = my1imgpro.o my1imgutil.o my1imgvid.o my1visdev.o my1vistest.o

CFLAGS += -Wall
LFLAGS += -lm
OFLAGS += -lavcodec -lavutil -lavformat -lSDL -lswscale -lavdevice
VFLAGS = -DMY1APP_PROGVERS=\"$(shell date +%Y%m%d)\"
DFLAGS =

RM = rm -f
CC = gcc -c
LD = gcc

ifeq ($(DO_DEBUG),YES)
	DFLAGS = -g -DMY1DEBUG
endif
debug: DFLAGS = -g -DMY1DEBUG

main: image

all: image video

image: $(TESTIMG)

video: $(TESTVIS)

new: clean main

debug: new

${TESTIMG}: $(OBJSIMG)
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS)

${TESTVIS}: $(OBJSVIS)
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS)

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(DFLAGS) $(VFLAGS) -o $@ $<

%.o: src/%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(VFLAGS) -o $@ $<

clean:
	-$(RM) $(TESTIMG) $(OBJSIMG) $(TESTVIS) $(OBJSVIS) *.o
