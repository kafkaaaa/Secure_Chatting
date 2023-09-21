#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "base64.h"

#define MSG_LEN_LIMIT 100
#define LEN_LIMIT 20
#define MAX_CLIENT_NUM 100
#define MAX_IP 30

struct tm *t;
extern int client_cnt;


// ANSI Escape: \033
// font code: [0;33m        {30, 31, 32, 33, 34, 35, 36, 37} = {Black, Red, Green, Yellow, Blue, Purple, Cyan, White}
// font reset: \033[0m
char* entrance_msg_font = "\033[0;33m";    // normal & yellow
char* exit_msg_font = "\033[0;31m";        // normal & red
char* reset_font = "\033[0m";              // return to default

