#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define MSG_LEN_LIMIT 100
#define LEN_LIMIT 20

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

void show_info();
void changeName();

char name[LEN_LIMIT];
char server_port[LEN_LIMIT];
char server_time[LEN_LIMIT];
char client_ip[LEN_LIMIT];
char msg_form[LEN_LIMIT]; 
char msg[MSG_LEN_LIMIT];


int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t send_thread, receive_thread;
    void *thread_return;

    if (argc != 4)
    {
        printf("ex) ./client [IP] [PORT] [NAME]\n");
        printf("ex) ./client 192.168.0.1 8080 홍길동\n");
        exit(1);
    }

    // C 현재 시간 사용하기 https://coding-factory.tistory.com/668
    struct tm *t;
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(server_time, "%d/%d/%d  %2d:%2d:%02d",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour + 9, t->tm_min, t->tm_sec);    // korea -> (UTC + 09:00)

    sprintf(name, "%s", argv[3]);
    sprintf(client_ip, "%s", argv[1]);
    sprintf(server_port, "%s", argv[2]);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect error");


    /** call show_info **/
    show_info();

    pthread_create(&send_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&receive_thread, NULL, recv_msg, (void *)&sock);
    pthread_join(send_thread, &thread_return);
    pthread_join(receive_thread, &thread_return);
    close(sock);
    return 0;
}


void *send_msg(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[LEN_LIMIT + MSG_LEN_LIMIT + 4];
    char my_info[MSG_LEN_LIMIT];
    char *who = NULL;
    char temp[MSG_LEN_LIMIT];

    printf("┌───────────── Eglobal Talk ─────────────┐\n");
    sprintf(my_info, " [%s]님이 입장하셨습니다 (%s)\n", name, client_ip);
    write(sock, my_info, strlen(my_info));

    while (1)
    {
        fgets(msg, MSG_LEN_LIMIT, stdin);

        // 채팅방 나감(종료) 처리
        if (strcmp(msg, "q\n") == 0 || strcmp(msg, "Q\n") == 0)
        {
            // printf("[name]")
            close(sock);
            exit(0);
        }

        // send message
        sprintf(name_msg, "[%s]: %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void *recv_msg(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[LEN_LIMIT + MSG_LEN_LIMIT];
    int str_len;

    while (1)
    {
        str_len = read(sock, name_msg, LEN_LIMIT + MSG_LEN_LIMIT - 1);
        if (str_len == -1)
            return (void *)-1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void show_info()
{
    // system("clear");
    printf("┌───────────── Client  Info ─────────────┐\n");
        // Test Code
        printf(" Server port  : %s \n", server_port);
        printf(" Client IP    : %s \n", client_ip);
        printf(" My Name      : %s \n", name);
        printf(" Current Time : %s \n", server_time);

    printf("└────────────────────────────────────────┘\n");
    printf("채팅을 종료하려면 q를 입력하세요\n\n");
    // TODO: 기능 추가 #1. 채팅 메시지 전송시 AES 암/복화
    // TODO: 기능 추가 #2. 채팅 로그 파일로 저장
    
    // TODO: 한 사용자(client)가 채팅방을 나갔을 때 메시지 표시하기 (현재 인원도 다시 카운팅)
    // 
}


void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

