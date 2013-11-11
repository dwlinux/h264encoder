SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
CC=arm-linux-gnueabi-gcc -g
CPP=arm-linux-gnueabi-g++
CFLAGS  =
LIBS	= -pthread -L. -lm -lrt ./linux_lib/libcedarv_osal.a ./linux_lib/libcedarxalloc.a ./linux_lib/libh264enc.a

all: simplerecorder

simplerecorder: $(OBJS)
	$(CPP) -Wall -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -Wall -c $< -o $@ $(CFLAGS)

clean:
	rm $(OBJS) simplerecorder
