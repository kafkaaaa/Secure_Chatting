CC = gcc
CFLAGS = -W
LDFLAGS = -lpthread

OBJ = server.o client.o
SRC = chat_server.c chat_client.c

all : server client

TARGET = server client base64.o

server : chat_server.c chat.h base64.o base64.h
	$(CC) $(CFLAGS) -o server $^ $(LDFLAGS)

client : chat_client.c chat.h base64.o base64.h
	$(CC) $(CFLAGS) -o client $^ $(LDFLAGS)

base64.o : base64.c base64.h
	$(CC) $(CFLAGS) -c base64.c

clean :
	rm -f *.o
	rm -f $(TARGET)

.PHONY : clean

