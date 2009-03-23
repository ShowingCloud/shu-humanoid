CC		= gcc
CFLAGS		= -I. -O0 -g -Wall -Wextra $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS		= $(shell pkg-config --libs gtk+-2.0)

obj_Visiond	= Visiond.o HSV.o FrameQueue.o ColorFileOper.o Paint.o Scatter_Spread.o ColorIdentify.o BottomLayer.o
obj_Console	= ConsoleGuarder.o
obj_Gtk		= GtkGuarder.o GtkFunc.o
obj_Socket	= SocketServer.o BottomLayer.o

all: Visiond ConsoleGuarder GtkGuarder SocketServer

%.o: %.c MyVision.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

Visiond: $(obj_Visiond)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Visiond)

ConsoleGuarder: $(obj_Console)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Console)

GtkGuarder: $(obj_Gtk)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Gtk)

SocketServer: $(obj_Socket)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Socket)

clean:
	rm -f $(obj_Visiond) $(obj_Console) $(obj_Gtk) $(obj_Socket) Visiond ConsoleGuarder GtkGuarder SocketServer
