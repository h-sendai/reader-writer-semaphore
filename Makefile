PROG = multibuf
CFLAGS = -g -pthread -O2 -Wall
LDFLAGS += -pthread

all: $(PROG)
OBJS += multibuf.o
OBJS += readn.o
OBJS += host_info.o
OBJS += reader.o
OBJS += writer.o
OBJS += my_socket.o
OBJS += get_num.o
OBJS += my_signal.o

multibuf.o: multibuf.h multibuf.c
reader.o: multibuf.h reader.c
writer.o: multibuf.h writer.c

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
