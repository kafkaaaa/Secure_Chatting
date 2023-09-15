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

int client_cnt = 0;
int client_sockets[MAX_CLIENT_NUM];
pthread_mutex_t mutex;

int main(int argc, char *argv[])
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
        printf(" Connceted client IP : %s ", inet_ntoa(client_addr.sin_addr));
        printf("(%d-%d-%d %d:%d)\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
               t->tm_hour, t->tm_min);
        printf(" (현재 인원: %d명)\n", client_cnt);
    }
    close(server_socket);
    return 0;
}

void *handle_cilent(void *arg)
{
    int client_socket = *((int *)arg);
    int str_len = 0, i;
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

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/types.h>
// #include <sys/socket.h>

// #define BUFF_SIZE 1024

// int main()
// {
//     int server_socket;
//     int client_socket;
//     int client_addr_size;

//     struct sockaddr_in server_addr;     // sockaddr 구조체 = 소켓의 주소체계, IP, Port 정보를 담고있는 구조체.
//     struct sockaddr_in client_addr;     // sockaddr_in = IPv4 소켓 주소 구조체

//     char buff_rcv[BUFF_SIZE + 5];
//     char buff_snd[BUFF_SIZE + 15];

//     // socket( ): 소켓 생성
//     // socket( 1. Domain(=Protocol family) / 2. Socket type / 3. Protocol )
//     // 1. Protocol_Family = { PF_INET(IPv4), PF_INET6(IPv6), PF_LOCAL, PF_PACKET, PF_IPX }
//     // 2. Socket Type = { SOCK_STREAM(연결형. TCP.), SOCK_DGRAM(비연결형. UDP.), SOCK_RAW }
//     // 3. Protocol = TCP/UDP경우 0을 넣어도 됨. Raw socket의 경우나 프로토콜이 여러개로 나눠지는 경우 사용함.
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (-1 == server_socket)
//     {
//         printf("server socket creation error\n");
//         exit(1);
//     }

//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     server_addr.sin_port = htons(1234);     // Any port number Unused
//     // htons = host to network short.
//     // -네트워크 통신의 Byte order는 Big Endian으로 정해져있어서 host의 Byte order를 Big Endian으로 변환해줌
//     // -INADDR_ANY(=0.0.0.0) = 자동으로 내 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미.
//     // TODO: 0.0.0.0과 127.0.0.1 의 차이점??

//     // bind( ): 소켓의 주소 할당 및 연결
//     // bind( Socket File-Descriptor / struct sockaddr *addr / addr_len )
//     // 성공: 0, 실패: -1
//     if (-1 == bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
//     {
//         printf("bind() error \n");
//         exit(1);
//     }

//     // listen( ): 소켓을 연결 가능 상태로 만듦
//     // listen( 1.Socket File-Descriptor / 2.backlog )
//     // 2.backlog = 연결을 위해 대기하는 Queue의 최대 길이 지정 (보통 5로 지정함)
//     if (-1 == listen(server_socket, 5))
//     {
//         printf("listen() error \n");
//         exit(1);
//     }

//     while (1)
//     {
//         client_addr_size = sizeof(client_addr);
//         // accept( ): 연결 요청을 수락함
//         // accept( 1.Socket File-Descriptor / 2.struct sockaddr *addr / 3.addr_len )
//         client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);

//         if (-1 == client_socket)
//         {
//             printf("클라이언트 연결 수락 실패\n");
//             exit(1);
//         }

//         read(client_socket, buff_rcv, BUFF_SIZE);
//         printf("receive: %s\n", buff_rcv);

//         sprintf(buff_snd, "%ld : %s", strlen(buff_rcv), buff_rcv);
//         write(client_socket, buff_snd, strlen(buff_snd) + 1); // +1: NULL까지 포함해서 전송
//         close(client_socket);
//     }
// }
