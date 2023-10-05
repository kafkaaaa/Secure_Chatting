#include "chat.h"
// #include <termios.h>    // to turn off echo in terminal

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);
void print_client_info();
void *send_exit_msg(int sock);
void *send_entrance_msg(int sock);
void make_entrance_msg(char* entrance_msg);
void make_exit_msg(char* exit_msg);
void encrypt_msg(uint8_t plain[], uint8_t result[]);
void clear_buffer();
// void int_handler(int sig);

char name[LEN_LIMIT];
char server_port[LEN_LIMIT];
char current_time[LEN_LIMIT];
char client_ip[LEN_LIMIT];
char msg_form[LEN_LIMIT]; 

int client_cnt;
int flag;
struct tm *t;


int main(int argc, char *argv[])
{
    // /* terminal echo off setting */
    // struct termios tm;
    // tcgetattr(STDIN_FILENO, &tm);
    // tm.c_lflag &= ~ECHO; // turn off echo
    // tcsetattr(STDIN_FILENO, TCSANOW, &tm);
    // /* */


    int sock;
    struct sockaddr_in serv_addr;
    pthread_t send_thread, recv_thread;
    void *thread_return;

    if (argc != 4) {
        printf("ex) %s [IP] [PORT] [NAME]\n", argv[0]);
        printf("ex) ./client 192.168.0.1 8080 홍길동\n");
        exit(1);
    }

    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(current_time, "%d/%d/%d  %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                                      t->tm_hour + 9,    t->tm_min,     t->tm_sec);

    sprintf(client_ip,      "%s", argv[1]);
    sprintf(server_port,    "%s", argv[2]);
    sprintf(name,           "%s", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect error");
    }

    print_client_info();

    pthread_create(&send_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&recv_thread, NULL, recv_msg, (void *)&sock);

    pthread_join(send_thread, &thread_return);
    pthread_join(recv_thread, &thread_return);

    close(sock);

    return 0;
}


/* 메시지 전송 */
void *send_msg(void *arg)
{
    uint8_t i;
    int sock = *((int *)arg);
    char msg[MSG_LEN_LIMIT];
    char dialog_msg[200];

    printf("┌───────────── Eglobal Talk ─────────────┐\n");
    send_entrance_msg(sock);

    while (1)
    {
        // fgets(msg, MSG_LEN_LIMIT, stdin);
        scanf(" %[^\n]s", msg);     // [^\n] = \n이 나오기 전 까지 모든 문자열을 받겠다는 의미.
        clear_buffer();
        strcat(msg, "\n");

        // client exit
        if (strcmp(msg, "X\n") == 0) {
            write(sock, NULL, 1);
            send_exit_msg(sock);
            break;
        }

        /* 일반 대화 메시지 전송(+암호화) 작업 */
        /* plain text -> #1. AES-256 Encryption -> Binary data -> #2. Base64 Encoding */
        sprintf(dialog_msg, "[%s]: %s", name, msg);   // 이름과 메시지 함께 인코딩+암호화

        // #1. AES-256 Encryption
        uint8_t* encrypted_msg = (uint8_t*) calloc(MSG_LEN_LIMIT + 16, sizeof(uint8_t));
        encrypt_msg(dialog_msg, encrypted_msg);
            printf("-----------------------------------------------------------------------\n");
            printf("[암호화 결과] = ");
            for (i=0; i<strlen(encrypted_msg); i++) {
                printf("%hhx", encrypted_msg[i]);
            }
            puts("");

        // #2. Base64 Encoding
        char* base64_msg = (char*) calloc(MSG_LEN_LIMIT + 16, sizeof(char));
        int ret = base64_encoder(encrypted_msg, strlen(encrypted_msg), base64_msg, MSG_LEN_LIMIT);
            if (ret <= 0) printf("[base64 encoding ERROR!!!]");
            else printf("[인코딩 결과] = %s\n", base64_msg);
            printf("-----------------------------------------------------------------------\n");

        write(sock, base64_msg, strlen(base64_msg));
        free(encrypted_msg);
        free(base64_msg);
    }

    // // client exit
    // write(sock, NULL, 1);
    // send_exit_msg(sock);

    return NULL;
}


// 처음 입장시 메시지
void *send_entrance_msg(int sock)
{
    char entrance_msg[MSG_LEN_LIMIT];
    make_entrance_msg(entrance_msg);

    write(sock, entrance_msg, strlen(entrance_msg));
}


// 클라이언트 퇴장 메시지
void *send_exit_msg(int sock)
{
    int len = 2 * LEN_LIMIT + 40;
    char exit_msg[len];
    make_exit_msg(exit_msg);
        // printf("\n[TEST] %s\n", exit_msg);

    write(sock, exit_msg, strlen(exit_msg));
    close(sock);

    exit(0);
}


/* 메시지 수신 */
void *recv_msg(void *arg)
{
    int msg_len;
    int sock = *((int *)arg);
    char recv_msg[200] = {0, };
    // char* recv_msg = (char*)calloc(200, sizeof(char));

    while (1)
    {
        // msg_len = read(sock, recv_msg, LEN_LIMIT + MSG_LEN_LIMIT - 1);
        msg_len = read(sock, recv_msg, 200 - 1);
        if (msg_len == -1) {
            return (void*)-1;
        }
        recv_msg[msg_len] = 0;
        fputs(recv_msg, stdout);
    }

    free(recv_msg);
    return NULL;
}


/* client 정보 출력 */
void print_client_info()
{
    printf("\n┌───────────── Client Info ─────────────┐\n");
        printf(" Server port  : %s \n", server_port);
        printf(" Client IP    : %s \n", client_ip);
        printf(" My Name      : %s \n", name);
        printf(" Current Time : %s \n", current_time);
    printf("└───────────────────────────────────────┘\n");
    printf("채팅방을 나가시려면 X를 눌러 조의를 표하십시오..\n\n");
}


/* 에러 처리 */
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}


void make_entrance_msg(char* entrance_msg)
{
    char* temp = (char*) calloc(MSG_LEN_LIMIT, sizeof(char));
    strcat(entrance_msg, entrance_msg_font);
    sprintf(temp, "※ [%s] (%s)님이 입장하셨습니다.", name, client_ip);
    strcat(entrance_msg, temp);
    free(temp);

    // add current time 
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(current_time, " (%d/%d/%d  %02d:%02d:%02d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                                           t->tm_hour + 9,    t->tm_min,     t->tm_sec);
    strcat(entrance_msg, current_time);
    strcat(entrance_msg, reset_font);
    strcat(entrance_msg, "\n");
}


void make_exit_msg(char* exit_msg)
{
    int len = strlen(exit_msg);
    char* temp = (char*) calloc(len + 40, sizeof(char));
    strcat(exit_msg, exit_msg_font);
    sprintf(temp, "※ [%s] (%s)님이 퇴장하셨습니다.", name, client_ip);
    strcat(exit_msg, temp);
    free(temp);

    // add current time 
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(current_time, " (%d/%d/%d  %02d:%02d:%02d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                                           t->tm_hour + 9,    t->tm_min,     t->tm_sec);
    strcat(exit_msg, current_time);
    strcat(exit_msg, reset_font);
    strcat(exit_msg, "\n");
}


/* AES-256, CBC 암호화 함수 */
void encrypt_msg(uint8_t plain[], uint8_t result[])
{
    size_t i;
    size_t plain_len = strlen(plain);
    size_t key_len = strlen(key);
    size_t hex_plain_len = plain_len;
    if (plain_len % 16) {
        hex_plain_len += 16 - (plain_len % 16);
    }

    size_t hex_key_len = key_len;
    if (key_len % 16) {
        hex_key_len += 16 - (key_len % 16);
    }

    uint8_t padded_plain[hex_plain_len];
    uint8_t padded_key[hex_key_len];
    memset(padded_plain, 0, hex_plain_len);
    memset(padded_key, 0, hex_key_len);

    for (i=0; i<plain_len; i++)
        padded_plain[i] = plain[i];
    
    for (i=0; i<key_len; i++)
        padded_key[i] = key[i];
    
    pkcs7_padding_pad_buffer(padded_plain, plain_len, sizeof(padded_plain), AES_BLOCKLEN);
    pkcs7_padding_pad_buffer(padded_key, key_len, sizeof(padded_key), AES_BLOCKLEN);
    
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, padded_key, iv);
    AES_CBC_encrypt_buffer(&ctx, padded_plain, hex_plain_len);

    memcpy(result, padded_plain, hex_plain_len);
}


void clear_buffer() {
    while (getchar() != '\n');
}


