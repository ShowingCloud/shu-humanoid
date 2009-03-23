CC		= arm-linux-gnu-gcc
CFLAGS		= -I. -O2 -Wall -Wextra -DHAS_MOTORS
LDFLAGS		= 
CP		= cp
PMOUNT		= pmount
PUMOUNT		= pumount

obj_Visiond	= Visiond.o FrameQueue.o ConfigFiles.o Paint.o ScatterSpread.o ColorIdentify.o BottomLayer.o
obj_Console	= ConsoleGuarder.o
#obj_Gtk		= GtkGuarder.o GtkFunc.o
obj_Socket	= SocketServer.o BottomLayer.o
obj_Motord	= Motord.o BottomLayer.o

Projs		= Visiond ConsoleGuarder SocketServer Motord
dest_disk	= sdb1
dest_dev	= /dev/$(dest_disk)
dest_dir	= /media/$(dest_disk)/prog/

all: $(Projs)

%.o: %.c %.h MyVision.h Makefile
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

install:
	$(PMOUNT) $(dest_dev)
	$(CP) -f $(Projs) $(dest_dir)
	$(PUMOUNT) $(dest_dev)

clean:
	rm -f $(obj_Visiond) $(obj_Console) $(obj_Gtk) $(obj_Socket) $(obj_Motord) $(Projs)
