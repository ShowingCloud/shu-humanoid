CC		= arm-linux-gnu-gcc
CFLAGS		= -I. -O3 -Wall -Wextra -Winline -finline-functions -DHAS_MOTORS -mtune=xscale -march=armv5te #-DVERBOSE
LDFLAGS		= -Wl,-O1 -Wl,-z,now -Wl,--sort-common -Wl,--hash-style=both -s -Wl,--as-needed
CP		= cp
PMOUNT		= pmount
PUMOUNT		= pumount
MKDIR		= mkdir

obj_Visiond	= Visiond.o QueueOper.o ConfigFiles.o Paint.o ScatterSpread.o ColorIdentify.o BottomLayer.o
obj_Console	= ConsoleGuarder.o BottomLayer.o
#obj_Gtk	= GtkGuarder.o GtkFunc.o
obj_Socket	= SocketServer.o BottomLayer.o
obj_Motord	= Motord.o BottomLayer.o

Projs		= Visiond ConsoleGuarder SocketServer Motord
Confs		= .colorsrc
dest_disk	= sdb1
#dest_dev	= /dev/$(dest_disk)
#dest_dir	= /media/$(dest_disk)/prog/
dest_dir	= /srv/tftp

all: $(Projs)

%.o: %.c %.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

Visiond: $(obj_Visiond) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Visiond)

ConsoleGuarder: $(obj_Console) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Console)

GtkGuarder: $(obj_Gtk) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Gtk)

SocketServer: $(obj_Socket) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Socket)

Motord: $(obj_Motord) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Motord)

install: all
#	$(PMOUNT) $(dest_dev)
#	$(MKDIR) -p $(dest_dir)
	$(CP) -f $(Projs) $(Confs) $(dest_dir)
#	$(PUMOUNT) $(dest_dev)

clean:
	rm -f $(obj_Visiond) $(obj_Console) $(obj_Gtk) $(obj_Socket) $(obj_Motord) $(Projs)
