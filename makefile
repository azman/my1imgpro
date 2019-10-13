# makefile for my1imgpro (basic image/vision processing library)
# - using libav & gtk for interfacing & display

TESTIMG = my1image_test
OBJSIMG = my1image.o my1image_util.o my1image_view.o
IMGFILE = my1image_file.o my1image_file_bmp.o my1image_file_pnm.o
IMGFILE += my1image_file_png.o
OBJSIMG += my1image_hist.o my1image_chsv.o $(IMGFILE)
OBJSIMG += my1image_work.o my1image_data.o $(TESTIMG).o
TESTVIS = my1video_test
OBJSVIS = my1image.o my1image_util.o my1image_work.o my1image_view.o $(IMGFILE)
OBJSVIS += my1video.o my1video_main.o $(TESTVIS).o
CHKSIZE = resizer
CHKLOAD = imgload
HSVTEST = testhsv
TOOLLST = $(CHKSIZE) $(CHKLOAD) $(HSVTEST)

CFLAGS += -Wall
LFLAGS += -lm
GFLAGS += $(shell pkg-config --libs gtk+-2.0)
OFLAGS += $(GFLAGS) -lavcodec -lavutil -lavformat -lswscale -lavdevice
VFLAGS = -DMY1APP_VERS=\"$(shell date +%Y%m%d)\"
DFLAGS =
TFLAGS += $(shell pkg-config --cflags gtk+-2.0)

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

$(CHKSIZE): my1image.o $(IMGFILE) my1image_resize.o
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

$(CHKLOAD): my1image.o $(IMGFILE) my1image_loader.o
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

$(HSVTEST): my1image.o my1image_chsv.o my1image_testhsv.o
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

${TESTIMG}: $(OBJSIMG)
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

${TESTVIS}: $(OBJSVIS)
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS)

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(DFLAGS) $(TFLAGS) -o $@ $<

%.o: src/%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(TFLAGS) $(VFLAGS) -o $@ $<

clean:
	-$(RM) $(TESTIMG) $(TESTVIS) $(TOOLLST) *.o *.pnm
