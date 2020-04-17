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
IMGVIEW = my1image_task.o my1image_view.o
IMGAPPW = $(IMGVIEW) my1image_appw.o my1image_hist.o my1image_data.o
IMGMONO = my1image_scan.o my1image_mono.o
IMGMNEW = my1image_grab.o my1image_main.o
OBJSIMG = $(IMGBASE) $(IMGFILE) $(IMGUTIL) $(IMGWORK) $(IMGAPPW) $(IMGMONO)
OBJSIMG += $(IMGMNEW) $(TESTIMG).o
TESTVIS = my1video_test
VISMAIN = my1video.o
VISNAME = $(subst .o,,$(VISMAIN))
VISAPPW = my1image_task.o my1image_view.o my1image_appw.o
VISBASE = my1video_base.o my1video_data.o
OBJSVIS = $(IMGBASE) $(IMGFILE) $(IMGUTIL) $(IMGWORK) $(VISAPPW) $(IMGMONO)
OBJSVIS += $(IMGMNEW) my1libav_grab.o $(VISBASE) $(TESTVIS).o
CHKSIZE = resizer
CHKLOAD = imgload
HSVTEST = testhsv
TOOLLST = $(CHKSIZE) $(CHKLOAD) $(HSVTEST)

CFLAGS += -Wall
LFLAGS += -lm
GFLAGS += $(shell pkg-config --libs gtk+-3.0)
OFLAGS += $(GFLAGS) -lavcodec -lavutil -lavformat -lswscale -lavdevice
VFLAGS = -DMY1APP_VERS=\"$(shell date +%Y%m%d)\"
DFLAGS =
TFLAGS += $(shell pkg-config --cflags gtk+-3.0)

RM = rm -f
CC = gcc -c
LD = gcc

ifeq ($(DO_DEBUG),YES)
	DFLAGS += -g -DMY1DEBUG
endif

# override flags for my1image bundle
test: DFLAGS = -D__MY1IMAGE_DO_MAIN__

$(CHKLOAD): DFLAGS = -D__MY1IMAGE_FILE_ONLY__

.PHONY: main test all image video new debug clean tools

main: image

test: chkimage.o my1libav_grab.o $(IMGMAIN)
	$(LD) $(CFLAGS) -o $(TESTIMG) $+ $(LFLAGS) $(GFLAGS) $(OFLAGS)

all: image video $(TOOLLST)

image: $(TESTIMG)

video: $(TESTVIS)

tools: $(TOOLLST)

new: clean main

debug: new

$(CHKSIZE): $(IMGBASE) $(IMGFILE) my1image_resize.o
	$(LD) $(CFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

$(CHKLOAD): $(IMGMAIN) my1image_loader.o
	$(LD) $(CFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

$(HSVTEST): $(IMGBASE) my1image_testhsv.o
	$(LD) $(CFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

${TESTIMG}: $(OBJSIMG)
	$(LD) $(CFLAGS) -o $@ $+ $(LFLAGS) $(GFLAGS)

${TESTVIS}: $(OBJSVIS)
	$(LD) $(CFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS)

$(IMGMAIN): src/$(IMGNAME).c src/$(IMGNAME).h
	$(CC) $(CFLAGS) $(DFLAGS) $(TFLAGS) -o $@ $<

$(VISMAIN): src/$(VISNAME).c src/$(VISNAME).h
	$(CC) $(CFLAGS) $(DFLAGS) $(TFLAGS) -o $@ $<

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(TFLAGS) -o $@ $<

%.o: src/%.c
	$(CC) $(CFLAGS) $(TFLAGS) $(VFLAGS) -o $@ $<

clean:
	-$(RM) $(TESTIMG) $(TESTVIS) $(TOOLLST) *.o
