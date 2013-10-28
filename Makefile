SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
# CC=arm-linux-gnueabi-gcc -g
CPP=g++
# CPP=arm-linux-gnueabi-g++
CC=gcc -g -O2
CFLAGS  = -Icedrus -Icedrus/mpeg-test
LIBS	= $(shell pkg-config --libs libavformat libavcodec libavutil)
OBJS += cedrus/common/*.o cedrus/mpeg-test/mpeg.o

all: simplerecorder

simplerecorder: $(OBJS)
	$(CPP) -Wall -o $@ $^ -pthread -L. -lm -lrt $(LIBS) ./linux_lib/libcedarv_osal.a ./linux_lib/libcedarxalloc.a ./linux_lib/libh264enc.a
%.o: %.c
	$(CC) -Wall -c $< -o $@ $(CFLAGS)

clean:
	rm $(OBJS) simplerecorder
