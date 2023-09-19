#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define MSG_LEN_LIMIT 100
#define MAX_CLIENT_NUM 100
#define MAX_IP 30

void *handle_cilent(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *msg);
char *server_state(int count);
void show_info(char port[]);

int reuse_opt = 1;
int client_cnt = 0;
int client_sockets[MAX_CLIENT_NUM];
pthread_mutex_t mutex;



int main(int argc, char *argv[])    // argc= argument count,   argv= argument value
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    // sockaddr 구조체 = 소켓의 주소체계, IP, Port 정보를 담고있는 구조체.
    // sockaddr_in = IPv4 소켓 주소 구조체

    int client_addr_len;
    pthread_t t_id;

    /** time log **/
    struct tm *t;
    time_t timer = time(NULL);
    t = localtime(&timer);

    if (argc != 2)
    {
        printf("ex) ./a.out [PORT]\n");
        printf("ex) ./server 8080\n");
        exit(1);
    }

    show_info(argv[1]);

    pthread_mutex_init(&mutex, NULL);
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    // socket( ): 소켓 생성
    // socket( 1. Domain(=Protocol family) / 2. Socket type / 3. Protocol )
    // 1. Protocol_Family = { PF_INET(IPv4), PF_INET6(IPv6), PF_LOCAL, PF_PACKET, PF_IPX }
    // 2. Socket Type = { SOCK_STREAM(연결형. TCP.), SOCK_DGRAM(비연결형. UDP.), SOCK_RAW }
    // 3. Protocol = TCP/UDP경우 0을 넣어도 됨. Raw socket의 경우나 프로토콜이 여러개로 나눠지는 경우 사용


    /* 현재 사용중인 IP주소와 포트번호 재사용: 서버 종료 후 재 실행시 bind() error 방지
    서버에서 연결을 해제해도 소켓은 바로 소멸되지 않고 Time-wait 상태를 거치는데, 
    이 때 최대 segment 수명의 2배를 기다린 후 CLOSED 상태로 전환된다. 따라서 서버가 연결을 종료하고
    바로 다시 실행하면 소켓이 Time-wait 상태에 있는 동안은 해당 PORT번호는 사용중이므로 bind error가 발생한다.
    https://url.kr/ia5gxb */
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt));


    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    // htons, htonl = host to network short/long
    // -네트워크 통신의 Byte order는 Big Endian으로 정해져있어서 host의 Byte order를 Big Endian으로 변환해줌
    // -INADDR_ANY(=0.0.0.0) = 자동으로 내 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미.
    // TODO: 0.0.0.0과 127.0.0.1 의 차이점??

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() error");
    if (listen(server_socket, 5) == -1)
        error_handling("listen() error");

    while (1)
    {
        t = localtime(&timer);
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        pthread_mutex_lock(&mutex);
        client_sockets[client_cnt++] = client_socket;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id, NULL, handle_cilent, (void *)&client_socket);
        pthread_detach(t_id);
        printf("Connceted client IP : %s ", inet_ntoa(client_addr.sin_addr));
        printf("(%d-%d-%d %02d:%02d:%02d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
               t->tm_hour + 9, t->tm_min, t->tm_sec + 1);
        printf("<현재 인원: %d명>\n", client_cnt);
    }
    close(server_socket);
    return 0;
}

void *handle_cilent(void *arg)
{
    int client_socket = *((int *)arg);
    int i, str_len = 0;
    char msg[MSG_LEN_LIMIT];

    while ((str_len = read(client_socket, msg, sizeof(msg))) != 0)
    send_msg(msg, str_len);

    // remove disconnected client
    pthread_mutex_lock(&mutex);
    for (i = 0; i < client_cnt; i++)
    {
        if (client_socket == client_sockets[i])
        {
            while (i++ < client_cnt - 1)
                client_sockets[i] = client_sockets[i + 1];
            break;
        }
    }
    client_cnt--;
    pthread_mutex_unlock(&mutex);
    close(client_socket);
    return NULL;
}


void send_msg(char *msg, int len)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < client_cnt; i++)
        write(client_sockets[i], msg, len);
    pthread_mutex_unlock(&mutex);
}


void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}


char *server_state(int count)
{
    char *stateMsg = malloc(sizeof(char) * 20);
    strcpy(stateMsg, "None");

    if (count < 5)
        strcpy(stateMsg, "Good");
    else
        strcpy(stateMsg, "Bad");

    return stateMsg;
}


void show_info(char port[])
{
    // system("clear");
    printf("┌────────── Chat Server Info ──────────┐\n");
    printf(" Port num of Server     = %s\n", port);
    printf(" State of Server        = %s\n", server_state(client_cnt));
    printf(" Max num of client      = %d\n", MAX_CLIENT_NUM);
    printf("└──────────────────────────────────────┘\n\n");
}

