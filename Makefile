CC		= gcc
CFLAGS		= -I. -O0 -g -Wall $(shell pkg-config --cflags gtk+-2.0 opencv)
LDFLAGS		= $(shell pkg-config --libs gtk+-2.0 opencv)

obj_Pickup	= My_Pickup.o HSV.o ColorFileOper.o GtkOper.o Paint.o
obj_Vision	= My_Vision.o HSV.o FrameQueue.o ColorFileOper.o GtkOper.o Paint.o Scatter_Spread.o ColorIdentify.o

all: My_Pickup My_Vision

%.o: %.c MyVision.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

My_Pickup: $(obj_Pickup)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Pickup)

My_Vision: $(obj_Vision)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Vision)

clean:
	rm -f *.o My_Pickup My_Vision
