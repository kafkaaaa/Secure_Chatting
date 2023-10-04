#include "chat.h"
#include <netinet/in.h>

void *handle_cilent(void *arg);
void send_msg(char *msg, int len);
void send_entrance_msg(char *msg);
void send_exit_msg(char *msg);
void error_handling(char *msg);
char *server_state(int count);
void print_server_info(char port[]);
void decrypt_msg(char* msg, char* result);

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

    chat_log_fp = fopen(CHAT_LOG_FNAME, "wb");
    if (chat_log_fp == NULL) {
        printf("[%s] 파일 생성 실패!\n", CHAT_LOG_FNAME);
    }
    fclose(chat_log_fp);

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
        printf("[New Connect!] client(%s)  ", inet_ntoa(client_addr.sin_addr));

        // add current time
        time_t timer = time(NULL);
        t = localtime(&timer);
        printf("(%d-%d-%d %02d:%02d:%02d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                              t->tm_hour + 9,    t->tm_min,     t->tm_sec);
                                              
        printf("<현재 인원: %d/%d명>\n\n", client_cnt, MAX_CLIENT_NUM);
    }

    close(server_socket);
    return 0;
}


void *handle_cilent(void *arg)
{
    int client_socket = *((int *)arg);
    int i, str_len = 0;
    char msg[200];

    // 처음 입장할 때 알림 메시지
    str_len = read(client_socket, msg, sizeof(msg));
    send_entrance_msg(msg);

    while (1) {
        str_len = read(client_socket, msg, sizeof(msg));

        if (msg == NULL) break;
        if (str_len == -1 || str_len == 0) {
            // printf("ERROR or EOF \n");
            break;
        }

        chat_log_fp = fopen(CHAT_LOG_FNAME, "ab");   // 로그파일 이어쓰기 (ab = append, binary) 
        fwrite(msg, sizeof(char), strlen(msg), chat_log_fp);
        fclose(chat_log_fp);
        send_msg(msg, str_len);
    }

    read(client_socket, msg, sizeof(msg));
    send_exit_msg(msg);

    // remove disconnected client
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
    printf("<현재 인원: %d/%d명>\n", client_cnt, MAX_CLIENT_NUM);

    pthread_mutex_unlock(&mutex);
    close(client_socket);
    return NULL;
}


void send_entrance_msg(char* msg)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i=0; i<client_cnt; i++) {      // broadcast to all clients
        write(client_sockets[i], msg, strlen(msg));
    }
    pthread_mutex_unlock(&mutex);
}


void send_exit_msg(char* msg)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i=0; i<client_cnt; i++) {
        write(client_sockets[i], msg, strlen(msg));
    }
    client_cnt--;
    pthread_mutex_unlock(&mutex);
}


void send_msg(char *msg, int len)
{
    int i;
    // size_t j;
    pthread_mutex_lock(&mutex);
    // 채팅방의 모든 client들에게 (복호화된) 메시지 전송
    // Base64 Encoded text -> #1. Base64 Decoding -> Binary -> #2. AES Decryption -> origin text
    for (i = 0; i < client_cnt; i++) {
      
        // #1. Base64 Decoding
        char base64_msg[MSG_LEN_LIMIT] = {0, };
        char decoded_msg[MSG_LEN_LIMIT] = {0, };
        char origin_msg[MSG_LEN_LIMIT] = {0, };
        size_t len = strlen(msg);
        base64_decoder(msg, len, decoded_msg, MSG_LEN_LIMIT);
            // printf("[디코딩 결과] = ");
            //     for (j=0; j<strlen(decoded_msg); j++) {
            //         printf("%hhx", decoded_msg[j]);
            //     }
            //     puts("");

        // #2. AES Decryption
        decrypt_msg(decoded_msg, origin_msg);
            // printf("[복호화 결과] = %s\n", origin_msg);

        write(client_sockets[i], origin_msg, strlen(origin_msg));
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
void decrypt_msg(char* decoded_msg, char* result)
{
    size_t i;
    size_t enc_len = strlen(decoded_msg);
        // test code
        // printf("복호화 대상 길이: %zd\n", enc_len);
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
            // printf("실제 평문 길이: %zd\n\n", actual_data_len);

        memcpy(result, decoded_msg, actual_data_len);
    }
    else {
        memcpy(result, decoded_msg, enc_len);
    }
}

