#include "chat.h"
// #include <termios.h>    // to turn off echo in terminal

void *send_msg(void *sock);
void *recv_msg(void *sock);
void print_client_info();
void *send_exit_msg(int sock);
void *send_entrance_msg(int sock);
void make_entrance_msg(char* msg);
void make_exit_msg(char* msg);
size_t encrypt_msg(uint8_t plain[], uint8_t result[]);
void add_current_time(char* str);
void clear_buffer();
void int_handler(int sig);

char name[LEN_LIMIT];
char server_port[LEN_LIMIT];
char current_time[LEN_LIMIT];
char client_ip[LEN_LIMIT]; 

int client_cnt;
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
        printf("[ERROR] connection error !!\n");
        exit(1);
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
    // int ret;
    int sock = *((int *)arg);
    char dialog_msg[MSG_LEN_LIMIT + 40];
    // char* dialog_msg = (char*) calloc(MSG_LEN_LIMIT * 2, sizeof(char));
    size_t i;

    printf("┌───────────────── Eglobal Talk ─────────────────┐\n");
    send_entrance_msg(sock);

    while (1)
    {
        // // catch 'Ctrl + C'
        // signal(sig, SIG_IGN);
        //         printf("ctrl c detected !!\n");
        

        char* msg = (char*)calloc(MSG_LEN_LIMIT + 1, sizeof(char));
        scanf(" %[^\n]s", msg);     // [^\n] = \n이 나오기 전 까지 모든 문자열을 받겠다는 의미.
        // ret = scanf(" %[^\n]s", msg);
        msg[strlen(msg)] = '\n';
        clear_buffer();

        if (strlen(msg) <= 0) {
            printf("[ERROR] msg input error !!\n");
            exit(1);
        }

        // client exit 처리
        // if (ret == EOF || strcmp(msg, "X\n") == 0) {
        if (strcmp(msg, "X\n") == 0) {
                // test code
                printf("\033[0;31m채팅을 종료합니다.\n\033[0m");
            break;
        }

        /* 일반 대화 메시지 전송(+암호화) 작업 */
        /* plain text -> #1. AES-256 Encryption -> Binary data -> #2. Base64 Encoding */
        sprintf(dialog_msg, "[%s]: %s", name, msg);   // 이름과 메시지 함께 인코딩+암호화

        // #1. AES-256 Encryption
        uint8_t* encrypted_msg = (uint8_t*) calloc(MSG_LEN_LIMIT + AES_BLOCKLEN, sizeof(uint8_t));
        size_t enc_len = encrypt_msg(dialog_msg, encrypted_msg);
                // // test code
                // printf("-----------------------------------------------------------------------");
                // printf("\033[0;32m\n[암호화 길이] = %zd\n", enc_len);
                // printf("[암호화 결과] = ");
                // for (i=0; i<enc_len; i++) {
                //     printf("%02hhx ", encrypted_msg[i]);
                // }
                // printf("\n-----------------------------------------------------------------------\n");

        // #2. Base64 Encoding
        char* base64_msg = (char*) calloc(MSG_LEN_LIMIT, sizeof(char));
        int msg_len = base64_encoder(encrypted_msg, enc_len, base64_msg, MSG_LEN_LIMIT);
                // test code
                if (msg_len <= 0) printf("[base64 encoding ERROR!!!]");
                // else {
                    // test code
                    // printf("[인코딩 길이] = %d\n", msg_len);
                    // printf("[인코딩 결과] = %s\n", base64_msg);
                // }
                // printf("-----------------------------------------------------------------------\n");

        base64_msg[msg_len] = msg_len;  // 길이 정보를 끝에 추가
                // test code
                // printf("[TEST] send msg length= %d...\n", (int)base64_msg[msg_len]);

        ssize_t written_bytes = write(sock, base64_msg, msg_len + 1);

                // test code
                if (written_bytes == -1) {
                    printf("[ERROR] send fail !!\n");
                }
                // else {
                    // printf("[TEST] send [%zd] bytes...\n\033[0m", written_bytes);
                // }
                // printf("-----------------------------------------------------------------------\n");

        free(encrypted_msg);
        free(base64_msg);
        // free(dialog_msg);
        free(msg);
    }

    // client exit
    send_exit_msg(sock);

    return NULL;
}


// 처음 입장시 메시지
void *send_entrance_msg(int sock)
{
    char* entrance_msg = (char*) calloc(MSG_LEN_LIMIT, sizeof(char));
    make_entrance_msg(entrance_msg);

    write(sock, entrance_msg, strlen(entrance_msg));
    free(entrance_msg);
}


// 클라이언트 퇴장 메시지
void *send_exit_msg(int sock)
{
    char* exit_flag = "exitflag";
    write(sock, exit_flag, strlen(exit_flag));

    // write(sock, NULL, 1);
    // if (write(sock, NULL, sizeof(NULL)) == -1) {
    //     puts("\n[ERROR] fail to send NULL !!\n");
    // }

    char* exit_msg = calloc(MSG_LEN_LIMIT + 40, sizeof(char));
    make_exit_msg(exit_msg);
    printf("%s", exit_msg);
            // test code
            // printf("\nexit_msg_len= %zd\nexit_msg= %s\n", strlen(exit_msg), exit_msg);

    write(sock, exit_msg, strlen(exit_msg));

    free(exit_msg);
    close(sock);

    exit(0);
}


/* 메시지 수신 */
void *recv_msg(void *arg)
{
    ssize_t i;
    int sock = *((int *)arg);

    while (1)
    {
        char* msg = (char*)malloc(MSG_LEN_LIMIT);
	    memset(msg, 0x00, MSG_LEN_LIMIT);
        ssize_t msg_len = read(sock, msg, MSG_LEN_LIMIT - 1);
        if (msg_len == -1) {
            printf("[ERROR] read msg fail!!\n");
            return (void*)-1;   // msg recv error
        }
        // msg[msg_len] = 0x00;       // EOF 표시
        //     // test code
        //     printf("\nmsg_len: %zd\n", msg_len);
        //     printf("strlen(msg)= %ld\n", strlen(msg));

        puts(msg);

        // TODO: 엄청난 white space가 출력되는 문제 !!!!
	    // printf("%s\n", msg );
        // puts(msg);
        // for (i=0; i<msg_len; i++) {     // 한 글자씩 (길이만큼) 출력하는데 sleep 걸어서 걸리는 시간 보기
        //     printf("%c", msg[i]);
        //     fflush(stdout);
        //     usleep(10000); // 0.01s
        // }

        free(msg);
    }

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


void add_current_time(char* str)
{
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(current_time, " (%d/%d/%d  %02d:%02d:%02d)"   , t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                                            t->tm_hour + 9,    t->tm_min,     t->tm_sec);
    strcat(str, current_time);
    strcat(str, reset_font);
    strcat(str, "\n");
}


void make_entrance_msg(char* entrance_msg)
{
    char* temp = (char*) calloc(MSG_LEN_LIMIT, sizeof(char));
    strcat(entrance_msg, entrance_msg_font);
    sprintf(temp, "※ [%s] (%s)님이 입장하셨습니다.", name, client_ip);
    strcat(entrance_msg, temp);
    free(temp);

    add_current_time(entrance_msg);
}


void make_exit_msg(char* exit_msg)
{
    int len = strlen(exit_msg);
    char* temp = (char*) calloc(len + 40, sizeof(char));
    strcat(exit_msg, exit_msg_font);
    sprintf(temp, "※ [%s] (%s)님이 퇴장하셨습니다.", name, client_ip);
    strcat(exit_msg, temp);
    free(temp);

    add_current_time(exit_msg);
}


/* AES-256, CBC 암호화 함수 */
size_t encrypt_msg(uint8_t plain[], uint8_t result[])
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


            // // test code
            // printf("-----------------------------------------------------------------------\n");
            // printf("[평문] =  %s", plain);
            // printf("plain_len= %zd\t key_len= %zd\t -> 패딩 이후: %zd\t %zd\n",
            //             plain_len, key_len, hex_plain_len, hex_key_len);


    for (i=0; i<plain_len; i++)
        padded_plain[i] = plain[i];
    
    for (i=0; i<key_len; i++)
        padded_key[i] = key[i];
    
    pkcs7_padding_pad_buffer(padded_plain, plain_len, sizeof(padded_plain), AES_BLOCKLEN);
    pkcs7_padding_pad_buffer(padded_key, key_len, sizeof(padded_key), AES_BLOCKLEN);
    
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, padded_key, iv);
    AES_CBC_encrypt_buffer(&ctx, padded_plain, hex_plain_len);

    for (i=0; i<hex_plain_len; i++) {
        result[i] = padded_plain[i];
    }
    
    return hex_plain_len;

    // memcpy(result, padded_plain, hex_plain_len);
}


void clear_buffer() {
    while (getchar() != '\n');
}


