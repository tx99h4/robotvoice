DIR      = $(HOME)/colibri/src/arm/devdep
#DIR      = $(HOME)/Bureau/المشروع/src/devdep
ROOTDIR  = $(HOME)/../uzumaki/colibri/rootfs/home/root

CC       = arm-linux-gnueabi-gcc
STRIP    = arm-linux-gnueabi-strip
CFLAGS   = -g -Wall
INCLUDES = -I$(DIR)/include -I$(DIR)/include/sphinxbase -I$(DIR)/include/pocketsphinx
LIBS     = -L$(DIR)/lib -lusb-0.1 -lnxt -lbluetooth -lsphinxbase -lpocketsphinx

all: main.o nxt.o btcomm.o rv.o
	$(CC) $(CFLAGS) -o main $^ $(LIBS)
	$(STRIP) main
#	cp -f main  $(ROOTDIR)
	cp -f main  /media/disk

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c main.c

nxt.o:  nxt.c nxt.h
	$(CC) $(CFLAGS) $(INCLUDES) -c nxt.c

btcomm.o:   btcomm.c bt.h
	$(CC) $(CFLAGS) $(INCLUDES) -c btcomm.c

rv.o:   rv.c rv.h nxt.h
	$(CC) $(CFLAGS) $(INCLUDES) -c rv.c

clean: 
	rm -f *.o *~ main
