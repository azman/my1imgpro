# makefile for my1imgpro (basic image/vision processing library)
# - using libav & gtk for interfacing & display

TESTIMG = my1image_test
IMGMAIN = my1image.o
IMGNAME = $(subst .o,,$(IMGMAIN))
IMGCORE = my1image_base.o my1image_argb.o
IMGBASE = $(IMGCORE) my1image_gray.o my1image_crgb.o my1image_chsv.o
IMGFILE = my1image_file.o my1image_file_bmp.o
IMGFILE += my1image_file_pnm.o my1image_file_png.o
IMGUTIL = my1image_mask.o my1image_area.o my1image_buff.o my1image_util.o
IMGWORK = my1image_stat.o my1image_work.o
IMGFULL = $(IMGBASE) $(IMGFILE) $(IMGUTIL) $(IMGWORK)
IMGAPPW = my1image_task.o my1image_view.o my1image_appw.o
IMGDATA = my1image_hist.o my1image_data.o
IMGMONO = my1image_scan.o my1image_mono.o
IMGMNEW = my1image_grab.o my1image_main.o
OBJSIMG = $(IMGFULL) $(IMGAPPW) $(IMGDATA) $(IMGMONO) $(IMGMNEW) $(TESTIMG).o
TESTVIS = my1video_test
VISMAIN = my1video.o
VISNAME = $(subst .o,,$(VISMAIN))
VISBASE = my1video_base.o my1video_data.o
OBJSVIS = $(IMGFULL) $(IMGAPPW) $(IMGMONO) $(IMGMNEW) my1libav_grab.o
OBJSVIS += $(VISBASE) $(TESTVIS).o
CHKSIZE = imgsize
CHKLOAD = imgload
HSVTEST = hsvtest
TOOLLST = $(CHKSIZE) $(CHKLOAD) $(HSVTEST)

CFLAGS += -Wall
LFLAGS += -lm
GFLAGC = $(shell pkg-config --cflags gtk+-3.0)
GFLAGL = $(shell pkg-config --libs gtk+-3.0)
AVFLAG = -lavcodec -lavutil -lavformat -lswscale -lavdevice
VFLAGS = -DMY1APP_VERS=\"$(shell date +%Y%m%d)\"
DFLAGS =

RM = rm -f
CC = gcc -c
LD = gcc

ifeq ($(DO_DEBUG),YES)
	DFLAGS += -g -DMY1DEBUG
endif

# override flags for my1image bundle
test: DFLAGS = -D__MY1IMAGE_DO_MAIN__
test: CFLAGS += $(GFLAGC)
test: LFLAGS += $(GFLAGL) $(AVFLAG)

loader: DFLAGS = -D__MY1IMAGE_FILE_ONLY__

resizer: DFLAGS = -D__MY1IMAGE_NO_HSV__ -D__MY1IMAGE_NO_VIEW__

testhsv: DFLAGS = -D__MY1IMAGE_NO_UTIL__ -D__MY1IMAGE_NO_VIEW__

.PHONY: main test all image video new debug clean tools loader resizer testhsv

main: image

test: chkimage.o my1libav_grab.o $(IMGMAIN)
	$(LD) $(CFLAGS) -o $(TESTIMG) $+ $(LFLAGS)

all: image video

image: $(TESTIMG)

video: $(TESTVIS)

tools: testhsv resizer loader

new: clean main

debug: new

resizer: $(IMGMAIN) $(CHKSIZE).o
	$(LD) $(CFLAGS) -o $(CHKSIZE) $+ $(LFLAGS)

loader: $(IMGMAIN) $(CHKLOAD).o
	$(LD) $(CFLAGS) -o $(CHKLOAD) $+ $(LFLAGS)

testhsv: $(IMGMAIN) $(HSVTEST).o
	$(LD) $(CFLAGS) -o $(HSVTEST) $+ $(LFLAGS)

${TESTIMG}: $(OBJSIMG)
	$(LD) $(CFLAGS) $(VFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGL)

${TESTVIS}: $(OBJSVIS)
	$(LD) $(CFLAGS) $(VFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGL) $(AVFLAG)

$(IMGMAIN): src/$(IMGNAME).c src/$(IMGNAME).h
	$(CC) $(CFLAGS) $(DFLAGS) -o $@ $<

$(VISMAIN): src/$(VISNAME).c src/$(VISNAME).h
	$(CC) $(CFLAGS) $(DFLAGS) -o $@ $<

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(GFLAGC) -o $@ $<

%.o: src/%.c
	$(CC) $(CFLAGS) $(GFLAGC) -o $@ $<

clean:
	-$(RM) $(TESTIMG) $(TESTVIS) $(TOOLLST) *.o
