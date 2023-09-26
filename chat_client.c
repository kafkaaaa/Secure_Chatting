#include "chat.h"

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);
void print_client_info();
void *send_exit_msg(int sock);
void *send_entrance_msg(int sock);
void make_entrance_msg(char* entrance_msg);
void make_exit_msg(char* exit_msg);
void encrypt_msg(uint8_t plain[], uint8_t result[]);

char name[LEN_LIMIT];
char server_port[LEN_LIMIT];
char current_time[LEN_LIMIT];
char client_ip[LEN_LIMIT];
char msg_form[LEN_LIMIT]; 
// char msg[MSG_LEN_LIMIT];

int client_cnt;
int flag;
struct tm *t;


int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t send_thread, receive_thread;
    void *thread_return;

    if (argc != 4)
    {
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

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect error");

    print_client_info();

    pthread_create(&send_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&receive_thread, NULL, recv_msg, (void *)&sock);
    // pthread_create(&ent_thread, NULL, send_ent_msg, (void *)&sock);
    // pthread_create(&exit_thread, NULL, send_exit_msg, (void *)&sock);
    pthread_join(send_thread, &thread_return);
    pthread_join(receive_thread, &thread_return);
    // pthread_join(ent_thread, &thread_return);
    // pthread_join(exit_thread, &thread_return);
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
        fgets(msg, MSG_LEN_LIMIT, stdin);

        // client exit
        if (strcmp(msg, "X\n") == 0) {
            write(sock, NULL, 1);
            send_exit_msg(sock);
            break;
        }


        /* 일반 대화 메시지 전송(+암호화) 작업 */
        /* plain text -> #1. AES-256 Encryption -> Binary data -> #2. Base64 Encoding */
        sprintf(dialog_msg, "[%s]: %s", name, msg);   // 이름과 메시지 함께 인코딩+암호화

        // TODO: #1. AES-256 Encryption
        uint8_t* encrypted_msg = (uint8_t*) calloc(MSG_LEN_LIMIT + 16, sizeof(uint8_t));
        encrypt_msg(dialog_msg, encrypted_msg);
            printf("-----------------------------------------------------------------------\n");
            printf("[복호화 결과] = ");
            for (i=0; i<strlen(encrypted_msg); i++) {
                printf("%hhx", encrypted_msg[i]);
            }
            puts("");

        // TODO: #2. Base64 Encoding
        char* base64_msg = (char*) calloc(MSG_LEN_LIMIT + 16, sizeof(char));
        // char base64_msg[MSG_LEN_LIMIT] = {0, };
        int ret = base64_encoder(encrypted_msg, strlen(encrypted_msg), base64_msg, MSG_LEN_LIMIT);
            if (ret <= 0) printf("[base64 encoding ERROR!!!]");
            else printf("[인코딩 결과] = %s\n", base64_msg);
            printf("-----------------------------------------------------------------------\n");

        write(sock, base64_msg, strlen(base64_msg));
        free(encrypted_msg);
        free(base64_msg);
        // write(sock, dialog_msg, strlen(dialog_msg));
    }

    send_exit_msg(sock);

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
    char exit_msg[MSG_LEN_LIMIT];
    make_exit_msg(exit_msg);
    // client_cnt--;
    write(sock, exit_msg, strlen(exit_msg));

    close(sock);
    exit(0);
}


/* 메시지 수신 */
void *recv_msg(void *arg)
{
    int sock = *((int *)arg);
    char dialog_msg[MSG_LEN_LIMIT + 32];
    int msg_len;

    while (1)
    {
        msg_len = read(sock, dialog_msg, LEN_LIMIT + MSG_LEN_LIMIT - 1);
        // msg_len = read(sock, dialog_msg, LEN_LIMIT + MSG_LEN_LIMIT);
        if (msg_len == -1) {
            return (void*)-1;
        }
        dialog_msg[msg_len] = 0;
        fputs(dialog_msg, stdout);
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

    // TODO: 기능 추가 #1. 채팅 로그 파일로 저장
    
}


/* 에러 처리 */
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}



/* client 연결(입장) 시 메시지 */
void make_entrance_msg(char* entrance_msg)
{
    char* temp = (char*) calloc(MSG_LEN_LIMIT, sizeof(char));
    strcat(entrance_msg, entrance_msg_font);
    sprintf(temp, "※ [%s] (%s)님이 입장하셨습니다.", name, client_ip);
    strcat(entrance_msg, temp);

    // add current time 
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(current_time, " (%d/%d/%d  %02d:%02d:%02d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                                           t->tm_hour + 9,    t->tm_min,     t->tm_sec);

    strcat(entrance_msg, current_time);
    strcat(entrance_msg, reset_font);
    strcat(entrance_msg, "\n");
}


/* client exit msg 작성 */
void make_exit_msg(char* exit_msg)
{
    char* temp = (char*) calloc(MSG_LEN_LIMIT, sizeof(char));

    strcat(exit_msg, exit_msg_font);
    sprintf(temp, "\n※ [%s] (%s)님이 퇴장하셨습니다.", name, client_ip);
    strcat(exit_msg, temp);

    // add current time 
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(current_time, " (%d/%d/%d  %02d:%02d:%02d)\n",
    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour + 9, t->tm_min, t->tm_sec);

    strcat(exit_msg, current_time);
    strcat(exit_msg, reset_font);
    strcat(exit_msg, "\n");
}


/* AES-256, CBC 암호화 함수 */
void encrypt_msg(uint8_t plain[], uint8_t result[])
{
    size_t i;
    // size_t plain_len = MSG_LEN_LIMIT;   // 고정크기로 암호화
    size_t plain_len = strlen(plain);
    size_t key_len = strlen(key);       // Length of Key
        // // Plain text TEST code
        // printf("[Plain Text] = ");
        // for (i=0; i<plain_len; i++)
        //     printf("%c", plain[i]);
        // printf("\n");

    // 평문 길이를 16의 배수로 맞춤
    size_t hex_plain_len = plain_len;
    if (plain_len % 16) {
        hex_plain_len += 16 - (plain_len % 16);
    }
        // printf("The original Length of (Plain Text) = %zd\n", plain_len);
        // printf("The Length of (padded Plain Text) = %zd\n", hex_plain_len);

    // 키의 길이를 16의 배수로 맞춤
    size_t hex_key_len = key_len;
    if (key_len % 16) {
        hex_key_len += 16 - (key_len % 16);
    }
        // printf("The original Length of (KEY) = %zd\n", key_len);
        // printf("The Length of (padded KEY) = %zd\n", hex_key_len);

    // 패딩된 데이터를 저장할 배열
    uint8_t padded_plain[hex_plain_len];
    uint8_t padded_key[hex_key_len];
    memset(padded_plain, 0, hex_plain_len);
    memset(padded_key, 0, hex_key_len);

    for (i=0; i<plain_len; i++)
        padded_plain[i] = plain[i];
    
    for (i=0; i<key_len; i++)
        padded_key[i] = key[i];
    
    /* padding with PKCS7 */
    // pkcs7_padding_pad_buffer -> returns the number of paddings it added
    pkcs7_padding_pad_buffer(padded_plain, plain_len, sizeof(padded_plain), AES_BLOCKLEN);
    pkcs7_padding_pad_buffer(padded_key, key_len, sizeof(padded_key), AES_BLOCKLEN);

    // printf("\nThe padded Plain Text (HEX) is...");
    // for (i=0; i<hex_plain_len; i++) {
    //     if (i % 16 == 0) puts("");
    //     printf("%02x ", padded_plain[i]);
    // }
    // puts("");

        // /* KEY padding TEST code */
        // printf("\nThe padded Key (HEX) is...");
        // for (i=0; i<hex_key_len; i++) {
        //     if (i % 16 == 0) puts("");
        //     printf("%02x ", padded_key[i]);
        // }
        // puts("");
    

    // ** Encryption **
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, padded_key, iv);
    AES_CBC_encrypt_buffer(&ctx, padded_plain, hex_plain_len);

    memcpy(result, padded_plain, hex_plain_len);

        // /* Encrypt TEST code */
        // printf("\n[Encrypted] String is... ");
        // for (i=0; i<hex_plain_len; i++) {
        //     if (i % 16 == 0) puts("");
        //     result[i] = padded_plain[i];
        //     printf("%02x ", padded_plain[i]);
        // }
        // puts("");

}


