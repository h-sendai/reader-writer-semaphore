PROG = reader-writer-semaphore
CFLAGS = -g -pthread -O2 -Wall
LDFLAGS += -pthread

all: $(PROG)
OBJS += reader-writer-semaphore.o
OBJS += readn.o
OBJS += host_info.o
OBJS += reader.o
OBJS += writer.o
OBJS += my_socket.o
OBJS += get_num.o
OBJS += my_signal.o

reader-writer-semaphore.o: reader-writer-semaphore.h reader-writer-semaphore.c
reader.o: reader-writer-semaphore.h reader.c
writer.o: reader-writer-semaphore.h writer.c

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
