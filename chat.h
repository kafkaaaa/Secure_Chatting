#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define MSG_LEN_LIMIT 100
#define MAX_CLIENT_NUM 100
#define MAX_IP 30

#define MSG_LEN_LIMIT 100
#define LEN_LIMIT 20

extern int client_cnt;


