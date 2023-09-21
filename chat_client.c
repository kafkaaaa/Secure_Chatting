#include "chat.h"

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

void print_client_info();
void changeName();

char name[LEN_LIMIT];
char server_port[LEN_LIMIT];
char server_time[LEN_LIMIT];
char client_ip[LEN_LIMIT];
char msg_form[LEN_LIMIT]; 
char msg[MSG_LEN_LIMIT];

int client_cnt;


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

    struct tm *t;
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(server_time, "%d/%d/%d  %02d:%02d:%02d",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour + 9, t->tm_min, t->tm_sec);    // (UTC + 09:00)

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
    pthread_join(send_thread, &thread_return);
    pthread_join(receive_thread, &thread_return);
    close(sock);
    return 0;
}


/* 메시지 전송 */
void *send_msg(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[LEN_LIMIT + MSG_LEN_LIMIT + 4];
    char exit_msg[LEN_LIMIT + MSG_LEN_LIMIT];
    char my_info[MSG_LEN_LIMIT];
    char *who = NULL;
    char temp[MSG_LEN_LIMIT];

    printf("┌───────────── Eglobal Talk ─────────────┐\n");
    sprintf(my_info, "※ [%s]님(%s)이 입장하셨습니다.\n", name, client_ip);
    write(sock, my_info, strlen(my_info));

    while (1)
    {
        fgets(msg, MSG_LEN_LIMIT, stdin);
        // msg[strlen(msg) - 1] = '\0';    // fgets( ) 개행문자 제거를 여기서 해버리면 일반 대화의 개행문자도 제거해버림

        // 채팅방 나감(종료) 처리
        // if (strcmp(msg, "X") == 0)
        if (strcmp(msg, "X\n") == 0)
        {
            sprintf(exit_msg, "※ [%s]님(%s)이 퇴장하셨습니다.\n", name, client_ip);
            client_cnt--;
            write(sock, exit_msg, strlen(exit_msg));
            close(sock);
            exit(0);
        }

        /* 메시지 전송 */
        sprintf(name_msg, "[%s]: %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}


/* 메시지 수신 */
void *recv_msg(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[LEN_LIMIT + MSG_LEN_LIMIT];
    int msg_len;

    while (1)
    {
        msg_len = read(sock, name_msg, LEN_LIMIT + MSG_LEN_LIMIT - 1);
        if (msg_len == -1) return (void *)-1;
        name_msg[msg_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}


void print_client_info()
{
    printf("\n┌───────────── Client Info ─────────────┐\n");
        printf(" Server port  : %s \n", server_port);
        printf(" Client IP    : %s \n", client_ip);
        printf(" My Name      : %s \n", name);
        printf(" Current Time : %s \n", server_time);

    printf("└────────────────────────────────────────┘\n");
    printf("채팅방을 나가시려면 X를 눌러 조의를 표하십시오..\n\n");

    // TODO: 기능 추가 #1. 채팅 메시지 전송시 AES 암/복화

    // TODO: 기능 추가 #2. 채팅 로그 파일로 저장
    
    // TODO: 한 사용자(client)가 채팅방을 나갔을 때 메시지 표시하기 (현재 인원도 다시 카운팅)
    
}


void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

