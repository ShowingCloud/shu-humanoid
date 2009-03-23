CC		= gcc
CFLAGS		= -I. -O2 -g -Wall -Wextra $(shell pkg-config --cflags gtk+-2.0) -DHAS_GTK -DVERBOSE
LDFLAGS		= $(shell pkg-config --libs gtk+-2.0)

obj_Visiond	= Visiond.o QueueOper.o ConfigFiles.o Paint.o ScatterSpread.o ColorIdentify.o BottomLayer.o
obj_Console	= ConsoleGuarder.o
obj_Gtk		= GtkGuarder.o GtkFunc.o
obj_Socket	= SocketServer.o BottomLayer.o
obj_Motord	= Motord.o BottomLayer.o

all: Visiond ConsoleGuarder GtkGuarder SocketServer Motord

%.o: %.c %.c Common.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

Visiond: $(obj_Visiond)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Visiond)

ConsoleGuarder: $(obj_Console)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Console)

GtkGuarder: $(obj_Gtk)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Gtk)

SocketServer: $(obj_Socket)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Socket)

Motord: $(obj_Motord)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Motord)

clean:
	rm -f $(obj_Visiond) $(obj_Console) $(obj_Gtk) $(obj_Socket) $(obj_Motord) Visiond ConsoleGuarder GtkGuarder SocketServer Motord
