# makefile for my1imgpro (basic image/vision processing library)
# - using libav & SDL for interfacing & display

RM = rm -f

CC      = gcc -c
LD      = gcc
CFLAG   = -Wall
LFLAG  = -lm
IFLAG  = -lSDL
VFLAG  = -lavcodec -lavutil -lavformat -lSDL -lswscale

TESTIMG = my1imgtest
OBJSIMG = my1imgpro.o my1imgutil.o my1imgmath.o my1imgfpo.o my1imgbmp.o my1imgpnm.o my1imgtest.o 
TESTVIS = my1vistest
OBJSVIS = my1imgpro.o my1imgvid.o my1visdev.o my1vistest.o

EXECUTE = $(TESTVIS)
OBJECTS = $(OBJSVIS)
debug: CFLAG += -g -DMY1DEBUG

all: $(TESTIMG) $(TESTVIS)

image: $(TESTIMG)

video: $(TESTVIS)

new: clean all

debug: new

${TESTIMG}: $(OBJSIMG)
	$(LD) $(CFLAG) -o $@ $+ $(LFLAG) $(VFLAG)

${TESTVIS}: $(OBJSVIS)
	$(LD) $(CFLAG) -o $@ $+ $(LFLAG) $(VFLAG)

%.o: src/%.c src/%.h
	$(CC) $(CFLAG) -o $@ $<

%.o: src/%.c
	$(CC) $(CFLAG) -o $@ $<

clean:
	-$(RM) $(TESTIMG) $(OBJSIMG) $(TESTVIS) $(OBJSVIS) *.o
