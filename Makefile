CC=gcc
CFLAGS=-I. -I/usr/include/opencv -O0 -g -Wall
LDFLAGS=-lcv -lhighgui -lcxcore -lml

SRCS	:= $(wildcard *.c)
OBJS	:= $(patsubst %.c, %, $(SRCS))

all: objs

objs: $(OBJS)
%: %.c MyVision.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)
