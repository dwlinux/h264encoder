SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
CC=arm-linux-gnueabi-gcc
CPP=arm-linux-gnueabi-g++
CFLAGS  =
LIBS	=

all: simplerecorder

simplerecorder: $(OBJS)
	$(CPP) -Wall -o $@ $^ $(LIBS) -static -pthread -L. -lm -lrt ./linux_lib/libcedarv_osal.a ./linux_lib/libcedarxalloc.a ./linux_lib/libh264enc.a
%.o: %.c
	$(CC) -Wall -c $< -o $@ $(CFLAGS)

clean:
	rm $(OBJS) simplerecorder
