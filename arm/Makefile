CC		= arm-linux-gnu-gcc
CFLAGS		= -I. -O2 -Wall -Wextra
LDFLAGS		= 

obj_Visiond	= Visiond.o FrameQueue.o ColorFileOper.o Paint.o Scatter_Spread.o ColorIdentify.o BottomLayer.o
obj_Console	= ConsoleGuarder.o
#obj_Gtk		= GtkGuarder.o GtkFunc.o
obj_Socket	= SocketServer.o BottomLayer.o
obj_Motors	= Motors.o BottomLayer.o

all: Visiond ConsoleGuarder SocketServer Motors

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

Motors: $(obj_Motors)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(obj_Motors)

clean:
	rm -f $(obj_Visiond) $(obj_Console) $(obj_Gtk) $(obj_Socket) $(obj_Motors) Visiond ConsoleGuarder GtkGuarder SocketServer Motors
