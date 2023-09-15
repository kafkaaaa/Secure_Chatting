CC = gcc
CFLAGS = -W
LDFLAGS = -lpthread

OBJ = server.o client.o
SRC = chat_server.c chat_client.c

all : server client

TARGET = server client

server : chat_server.c
	$(CC) $(CFLAGS) -o server $^ $(LDFLAGS)

client : chat_client.c
	$(CC) $(CFLAGS) -o client $^ $(LDFLAGS)

clean :
	rm -f *.o
	rm -f $(TARGET)

.PHONY : clean

