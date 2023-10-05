CC = gcc
CFLAGS = -W
LDFLAGS = -lpthread

OBJ = AES.o pkcs7_padding.o base64.o
SRC = chat_server.c chat_client.c
HEADER = chat.h AES.h base64.h pkcs7_padding.h
LOGFILE = log.bin

TARGET = server client

all : server client

server : chat_server.c chat.h $(OBJ)
	$(CC) $(CFLAGS) -o server $^ $(LDFLAGS)

client : chat_client.c chat.h $(OBJ)
	$(CC) $(CFLAGS) -o client $^ $(LDFLAGS)

AES.o : AES.c AES.h
	$(CC) $(CFLAGS) -c AES.c

pkcs7_padding.o : pkcs7_padding.c pkcs7_padding.h
	$(CC) $(CFLAGS) -c pkcs7_padding.c

base64.o : base64.c base64.h
	$(CC) $(CFLAGS) -c base64.c

clean :
	rm -f *.o
	rm -f $(TARGET)
	rm -f $(LOGFILE)

.PHONY : clean

