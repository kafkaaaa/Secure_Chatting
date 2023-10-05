#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "AES.h"
#include "pkcs7_padding.h"
#include "base64.h"


#define MSG_LEN_LIMIT 128
#define LEN_LIMIT 20
#define MAX_CLIENT_NUM 100
#define MAX_IP 30
#define IV_LEN 16
#define CHAT_LOG_FNAME "log.bin"

const uint8_t iv[IV_LEN] = {0x65, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x6d, 0x0d, 0x0a};
const char* key = "eglobalsystemeglobalsystem";


struct tm *t;
extern int client_cnt;


// ANSI Escape: \033
// font code: [0;33m        {30, 31, 32, 33, 34, 35, 36, 37} = {Black, Red, Green, Yellow, Blue, Purple, Cyan, White}
// font reset: \033[0m
// const char* entrance_msg_font = "\033[0;33m";    // normal & yellow
// const char* exit_msg_font = "\033[0;31m";        // normal & red
// const char* reset_font = "\033[0m";              // return to default

#define entrance_msg_font "\033[0;33m"    // normal & yellow
#define exit_msg_font "\033[0;31m"        // normal & red
#define reset_font "\033[0m"              // return to default

