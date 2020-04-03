# makefile for my1imgpro (basic image/vision processing library)
# - using libav & gtk for interfacing & display

TESTIMG = my1image_test
IMGBASE = my1image_base.o my1image_gray.o my1image_crgb.o my1image_chsv.o
IMGFILE = my1image_file.o my1image_file_bmp.o
IMGFILE += my1image_file_pnm.o my1image_file_png.o
IMGUTIL = my1image_mask.o my1image_area.o my1image_buff.o my1image_util.o
IMGWORK = my1image_stat.o my1image_work.o
IMGAPPW = my1image_view.o my1image_appw.o my1image_hist.o my1image_data.o
IMGMONO = my1image_scan.o my1image_mono.o
OBJSIMG = $(IMGBASE) $(IMGFILE) $(IMGUTIL) $(IMGWORK) $(IMGAPPW)
OBJSIMG += $(IMGMONO) $(TESTIMG).o
TESTVIS = my1video_test
OBJSVIS = $(IMGBASE) $(IMGFILE) $(IMGUTIL) $(IMGWORK)
OBJSVIS += my1image_view.o my1image_appw.o my1video.o my1video_main.o
OBJSVIS += $(TESTVIS).o
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
	DFLAGS += -g -DMY1DEBUG
endif

#test: DFLAGS += -D__MYIMAGE_NO_HSV__ -D__MYIMAGE_NO_FILE__
test: DFLAGS += -D__MYIMAGE_FILE_PNG__ -D__MYIMAGE_WORK__
test: DFLAGS += -D__MYIMAGE_VIEW__ -D__MYIMAGE_APPW__

.PHONY: main test all image video new debug clean

main: image

test: my1image.o chkimage.o
	$(LD) $(CFLAGS) $(DFLAGS) -o $(TESTIMG) $+ $(LFLAGS) $(GFLAGS)

all: image video $(TOOLLST)

image: $(TESTIMG)

video: $(TESTVIS)

new: clean main

debug: new

$(CHKSIZE): $(IMGBASE) $(IMGFILE) my1image_resize.o
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

$(CHKLOAD): $(IMGBASE) $(IMGFILE) my1image_loader.o
	$(LD) $(CFLAGS) $(DFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

$(HSVTEST): $(IMGBASE) my1image_testhsv.o
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
