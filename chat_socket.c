#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFF_SIZE 1024

int main()
{
    int server_socket;
    int client_socket;
    int client_addr_size;

    struct sockaddr_in server_addr;     // sockaddr 구조체 = 소켓의 주소체계, IP, Port 정보를 담고있는 구조체.
    struct sockaddr_in client_addr;     // sockaddr_in = IPv4 소켓 주소 구조체

    char buff_rcv[BUFF_SIZE + 5];
    char buff_snd[BUFF_SIZE + 5];


    // socket( ): 소켓 생성
    // socket( 1. Domain(=Protocol family) / 2. Socket type / 3. Protocol )
    // 1. Protocol_Family = { PF_INET(IPv4), PF_INET6(IPv6), PF_LOCAL, PF_PACKET, PF_IPX }
    // 2. Socket Type = { SOCK_STREAM(연결형. TCP.), SOCK_DGRAM(비연결형. UDP.), SOCK_RAW }
    // 3. Protocol = TCP/UDP경우 0을 넣어도 됨. Raw socket의 경우나 프로토콜이 여러개로 나눠지는 경우 사용함.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == server_socket)
    {
        printf("server socket creation error\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1234);     // Any unused port number
    // htons = host to network short.
    // -네트워크 통신의 Byte order는 Big Endian으로 정해져있어서 host의 Byte order를 Big Endian으로 변환해줌
    // -INADDR_ANY(=0.0.0.0) = 자동으로 내 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미.
    // TODO: 0.0.0.0과 127.0.0.1 의 차이점??


    // bind( ): 소켓의 주소 할당 및 연결
    // bind( Socket File-Descriptor / struct sockaddr *addr / addr_len )
    // 성공: 0, 실패: -1 
    if (-1 == bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        printf("bind() error \n");
        exit(1);
    }


    // listen( ): 소켓을 연결 가능 상태로 만듦
    // listen( 1.Socket File-Descriptor / 2.backlog )
    // 2.backlog = 연결을 위해 대기하는 Queue의 최대 길이 지정 (보통 5로 지정함)
    if (-1 == listen(server_socket, 5))
    {
        printf("listen() error \n");
        exit(1);
    }

    while (1)
    {
        client_addr_size = sizeof(client_addr);
        // accept( ): 연결 요청을 수락함
        // accept( 1.Socket File-Descriptor / 2.struct sockaddr *addr / 3.addr_len )
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);

        if (-1 == client_socket)
        {
            printf("클라이언트 연결 수락 실패\n");
            exit(1);
        }

        read(client_socket, buff_rcv, BUFF_SIZE);
        printf("receive: %s\n", buff_rcv);

        sprintf(buff_snd, "%d : %s", strlen(buff_rcv), buff_rcv);
        write(client_socket, buff_snd, strlen(buff_snd) + 1); // +1: NULL까지 포함해서 전송
        close(client_socket);
    }
}