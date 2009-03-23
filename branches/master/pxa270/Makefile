CC		= arm-linux-gnu-gcc
CFLAGS		= -I. -O3 -Wall -Wextra -DHAS_MOTORS -mtune=xscale -march=armv5te -DVERBOSE
LDFLAGS		=

obj_Visiond	= Visiond.o QueueOper.o ConfigFiles.o Paint.o ScatterSpread.o ColorIdentify.o BottomLayer.o
obj_Console	= ConsoleGuarder.o BottomLayer.o
obj_Decision	= Decision.o BottomLayer.o
obj_Motord	= Motord.o BottomLayer.o

all: Visiond ConsoleGuarder Decision Motord

%.o: %.c %.h Common.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

Visiond: $(obj_Visiond)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

ConsoleGuarder: $(obj_Console)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

Decision: $(obj_Decision)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

Motord: $(obj_Motord)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: all
	pmount /dev/sdc1
	sudo cp Visiond ConsoleGuarder Decision Motord .colorsrc /media/sdc1/prog
	sudo cp -a ../pace /media/sdc1/prog/
	pumount /dev/sdc1

clean:
	rm -f $(obj_Visiond) $(obj_Console) $(obj_Decision) $(obj_Motord) Visiond ConsoleGuarder Decision Motord
