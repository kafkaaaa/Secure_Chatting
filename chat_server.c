#include "chat.h"
#include <netinet/in.h>

void *handle_cilent(void *arg);
// void send_msg(char *msg, size_t msg_len);
void send_msg(char *msg);
void send_entrance_msg(char *msg);
void send_exit_msg(char *msg);
void error_handling(char *msg);
char *server_state(int count);
void print_server_info(char port[]);
void decrypt_msg(char* msg, int len, char* result);

int client_sockets[MAX_CLIENT_NUM];
int client_cnt;
int flag;
FILE* chat_log_fp;  // 채팅 로그 저장
pthread_mutex_t mutex;


int main(int argc, char *argv[])    // argc= argument count,   argv= argument value
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    // sockaddr 구조체 = 소켓의 주소체계, IP, Port 정보를 담고있는 구조체.
    // sockaddr_in = IPv4 소켓 주소 구조체

    int client_addr_len;
    pthread_t t_id;

    // struct tm *t;
    // time_t timer = time(NULL);
    // t = localtime(&timer);

    if (argc != 2)
    {
        printf("ex) %s [PORT]\n", argv[0]);
        printf("ex) ./server 8080\n");
        exit(1);
    }

    print_server_info(argv[1]);
    chat_log_fp = fopen(CHAT_LOG_FNAME, "wt");

    // Mutex는 화장실이 1개 있는 상황과 비슷하다.
    pthread_mutex_init(&mutex, NULL);

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    // socket( ) 소켓 생성
        // socket( 1.Domain(=Protocol family) / 2.Socket type / 3.Protocol )
        // 1. Protocol_Family   = { PF_INET(IPv4), PF_INET6(IPv6), PF_LOCAL, PF_PACKET, PF_IPX }
        // 2. Socket Type       = { SOCK_STREAM(연결형. TCP), SOCK_DGRAM(비연결형. UDP), SOCK_RAW }
        // 3. Protocol          = TCP/UDP경우 0을 넣어도 됨. Raw socket의 경우나 프로토콜이 여러 개로 나눠지는 경우에 사용


    /* 현재 사용중인 IP주소와 포트번호 재사용: 서버 종료 후 재 실행시 Bind error 방지 */
        // 서버에서 연결을 해제해도 소켓은 바로 소멸되지 않고 Time-wait 상태를 거치는데, 
        // 이 때 최대 Segment 수명의 2배를 기다린 후 Closed 상태로 전환된다. 따라서 서버가 연결을 종료하고 바로 다시 실행하면
        // 소켓이 Time-wait 상태에 있는 동안은 해당 포트는 사용중이므로 Bind error가 발생한다.
    int reuse_opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    // htons, htonl = host to network short/long
        // 네트워크 통신의 Byte order는 Big Endian으로 정해져있어서 host의 Byte order를 Big Endian으로 변환해줌
        // INADDR_ANY(=0.0.0.0) = 자동으로 내 컴퓨터에 존재하는 랜 카드 중 사용 가능한 랜 카드의 IP주소를 사용하라는 의미.

    // 소켓에 IP, 포트번호 할당
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("[bind] ERROR");
    }

    // 소켓 Listen
    if (listen(server_socket, 5) == -1) {
        error_handling("[listen] ERROR");
    }

    while (1) {
        // (스레드 = Listen 소켓, accept( )로 연결 요청 수락)
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) continue;

        pthread_mutex_lock(&mutex);
        client_sockets[client_cnt++] = client_socket;   // client 소켓 등록
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, handle_cilent, (void *)&client_socket);
        pthread_detach(t_id);

                                            // inet_nota  = IPv4 -> ASCII
        printf("[New Connect !] client(%s)  ", inet_ntoa(client_addr.sin_addr));

        // add current time
        time_t timer = time(NULL);
        t = localtime(&timer);
        printf("(%d-%d-%d %02d:%02d:%02d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                              t->tm_hour + 9,    t->tm_min,     t->tm_sec);
                                              
        printf("[현재 인원: %d / %d명]\n\n", client_cnt, MAX_CLIENT_NUM);
    }

    close(server_socket);
    fclose(chat_log_fp);
    return 0;
}


void *handle_cilent(void *arg)
{
    int client_socket = *((int *)arg);
    int i, msg_len = 0;
    char msg[200];
    chat_log_fp = fopen(CHAT_LOG_FNAME, "at");   // append


    // 처음 입장할 때 알림 메시지
    msg_len = read(client_socket, msg, sizeof(msg));
    send_entrance_msg(msg);
    if (fwrite(msg, sizeof(char), strlen(msg), chat_log_fp) != strlen(msg)) {
        printf("[ERROR] write to log file\n");
    }


    // 메시지 받아서 전송
    while (1) {
        char* normal_msg = (char*)calloc(MSG_LEN_LIMIT, sizeof(char));
        msg_len = read(client_socket, normal_msg, MSG_LEN_LIMIT - 1);
                // // test code
                // printf("[TEST] msg_len= %d\n", msg_len);
        // if (normal_msg == NULL) {   // client exit
        // if (normal_msg == "exit") {   // client exit


        // if (msg_len < AES_BLOCKLEN) {   // client exit
        if (strncmp(normal_msg, "exit", 4) == 0) {   // client exit
                // test code
                printf("[TEST] client exit occur !\n");
            break;
        }
        if (msg_len == -1 || msg_len == 0) { // ERROR or EOF
            break;
        }


                // printf("[일반 대화 전달]");
        send_msg(normal_msg);
        chat_log_fp = fopen(CHAT_LOG_FNAME, "ab");   // append      // TODO:
        fwrite(normal_msg, sizeof(char), msg_len, chat_log_fp);
        
        free(normal_msg);
    }


    // 클라이언트 퇴장 처리
    char* exit_msg = (char*)calloc(200, sizeof(char));
    msg_len = read(client_socket, exit_msg, MSG_LEN_LIMIT - 1);
    printf("%s\n", exit_msg);
    if (msg_len <= 0) {
        printf("\n**message read ERROR**\n");
    }
    else {
            // // test code
            // printf("exit_msg_len= %d, exit_msg= %s\n", msg_len, exit_msg);
        send_exit_msg(exit_msg);
        fwrite(msg, sizeof(char), strlen(exit_msg), chat_log_fp);
    }
    free(exit_msg);


    pthread_mutex_lock(&mutex);
    for (i = 0; i < client_cnt; i++)
    {
        if (client_socket == client_sockets[i])
        {
            while (i++ < client_cnt - 1) {
                client_sockets[i] = client_sockets[i + 1];
            }
            break;
        }
    }
    printf("[현재 인원: %d / %d명]\n\n", client_cnt, MAX_CLIENT_NUM);
    pthread_mutex_unlock(&mutex);

    close(client_socket);

    return NULL;
}


void send_entrance_msg(char* msg)
{
    int i;
    int len = strlen(msg);
    pthread_mutex_lock(&mutex);
    for (i=0; i<client_cnt; i++) {      // broadcast to all clients
        write(client_sockets[i], msg, len);
    }
    pthread_mutex_unlock(&mutex);
}


void send_exit_msg(char* msg)
{
    int i;
    int len = strlen(msg);
        // // test code
        // printf("exit msg: %s\nlen: %d\n", msg, len);
    pthread_mutex_lock(&mutex);
    for (i=0; i<client_cnt; i++) {      // broadcast to all clients
        write(client_sockets[i], msg, len);
    }
    client_cnt--;
    // printf("%s\n", msg);
    pthread_mutex_unlock(&mutex);
}


// void send_msg(char *msg, size_t msg_len)
void send_msg(char *msg)
{
    int i, j;
    pthread_mutex_lock(&mutex);

    // Base64 Encoded text -> #1. Base64 Decoding -> Binary -> #2. AES Decryption -> origin text
        // #1. Base64 Decoding
        char base64_msg[MSG_LEN_LIMIT] = {0, };
        char decoded_msg[MSG_LEN_LIMIT] = {0, };
        char origin_msg[MSG_LEN_LIMIT] = {0, };

        int decoded_len = base64_decoder(msg, strlen(msg), decoded_msg, MSG_LEN_LIMIT);
        // base64_decoder(msg, msg_len, decoded_msg, MSG_LEN_LIMIT);
                // // test code
                // printf("\n-----------------------------------------------------------------------\n");
                // // printf("\033[0;32m[디코딩 대상] = %s\n", decoded_msg);
                // printf("\033[0;32m[디코딩 대상] = %s\n", msg);
                // printf("[디코딩 결과] = ");
                // // for (j=0; j<strlen(decoded_msg); j++) {
                // for (j=0; j<decoded_len; j++) {
                //     printf("%02hhx ", decoded_msg[j]);
                // }
                // puts("");



        // #2. AES Decryption
        decrypt_msg(decoded_msg, decoded_len, origin_msg);
                // // test code
                // // printf("[복호화 길이] = %zd\n", strlen(decoded_msg));
                // printf("[복호화 길이] = %d\n", decoded_len);
                // // if (strlen(origin_msg) == 0) {
                // if (origin_msg == NULL) {
                //     printf("\033[1;31m[복호화 오류 !!!]\n");
                // }
                // else {
                //     // printf("[test] origin_msg_len = %zd\n", strlen(origin_msg));
                //     printf("[복호화 결과] = %s\033[0m", origin_msg);
                // }
                // printf("\n-----------------------------------------------------------------------\n");

    // 채팅방의 모든 client들에게 (복호화된) 메시지 전송
    for (i = 0; i < client_cnt; i++) {
        // write(client_sockets[i], origin_msg, strlen(origin_msg));
        write(client_sockets[i], origin_msg, decoded_len);
    }
    pthread_mutex_unlock(&mutex);
}


void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}


char* server_state(int count)
{
    char *state = calloc(7, sizeof(char));
    strcpy(state, "Vacant");

    if (count < MAX_CLIENT_NUM * 0.5) strcpy(state, "Good");
    else if (count < MAX_CLIENT_NUM * 0.7) strcpy(state, "So so");
    else strcpy(state, "Bad");

    return state;
}


void print_server_info(char* port)
{
    printf("\n┌────────── Chat Server Info ──────────┐\n");
    printf(" Port num of Server     = %s\n", port);
    printf(" State of Server        = %s\n", server_state(client_cnt));
    printf(" Max num of client      = %d\n", MAX_CLIENT_NUM);
    printf("└──────────────────────────────────────┘\n\n");
}


/* AES Decryption */
void decrypt_msg(char* decoded_msg, int decoded_msg_len, char* result)
{
    size_t i;
    // size_t enc_len = strlen(decoded_msg);    // 여기가 문제!! -> 암호화 결과에 00 (0x00)이 있으면 strlen에서 멈춰버림.
    size_t enc_len = decoded_msg_len;           // strlen대신 함수 인자로 길이를 받아버려서 해결
    size_t key_len = strlen(key);

    size_t hex_key_len = key_len;
    if (key_len % 16) {
        hex_key_len += 16 - (key_len % 16);
    }

    uint8_t padded_key[hex_key_len];
    memset(padded_key, 0, hex_key_len);
    for (i=0; i<key_len; i++) {
        padded_key[i] = (uint8_t)key[i];
    }

    pkcs7_padding_pad_buffer(padded_key, key_len, sizeof(padded_key), AES_BLOCKLEN);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, padded_key, iv);
    AES_ctx_set_iv(&ctx, iv);
    AES_CBC_decrypt_buffer(&ctx, decoded_msg, enc_len);

    if (enc_len % 16 != 0) {
        size_t actual_data_len = pkcs7_padding_data_length(decoded_msg, enc_len, AES_BLOCKLEN);
        memcpy(result, decoded_msg, actual_data_len);
    }
    else {
        memcpy(result, decoded_msg, enc_len);
    }
}

