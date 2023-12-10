#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SENDER_EMAIL "0819ljh@naver.com"
#define SENDER_PASSWORD "lock7136@"
#define RECIPIENT_EMAIL "sarah123456w1@gmail.com"
#define SUBJECT "이메일 제목"
#define BODY "이메일 본문 내용"

int sock; // 전역

void error_handling(const char *message, int sock) {
    close(sock);
    perror(message);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in server_addr; 
    char picture;
    ssize_t read_stat; // 루프 내부에서 선언

    if (argc != 3) { // client 프로그램 실행 시 인자에 IP랑 port num을 제대로 주지 않았을 때 
        printf("Please deliver IP & Port num as arguments Correctly!\n");
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0); // socket 생성 TCP 방식 IPv4
    if (sock == -1) 
        error_handling("socket() error", sock); // socket 생성시 error control

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 변환해서 저장
    server_addr.sin_port = htons(atoi(argv[2])); //port num 변환해서 저장

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
        error_handling("connect() error", sock); 

    while (1) {
        read_stat = read(sock, &picture, 1); // 여기서만 선언
        if (read_stat == -1) {
            perror("Client Output; reading data from socket error");
            exit(1);
        } else if (read_stat == 0) {
            break;
        } else {
            if (!strncmp(#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SENDER_EMAIL "0819ljh@naver.com"
#define SENDER_PASSWORD "lock7136@"
#define RECIPIENT_EMAIL "sarah123456w1@gmail.com"
#define SUBJECT "이메일 제목"
#define BODY "이메일 본문 내용"

int sock; // 전역

void error_handling(const char *message, int sock) {
    close(sock);
    perror(message);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in server_addr; 
    char picture;
    char gas_check='0';
    ssize_t read_stat; // 루프 내부에서 선언

    if (argc != 3) { // client 프로그램 실행 시 인자에 IP랑 port num을 제대로 주지 않았을 때 
        printf("Please deliver IP & Port num as arguments Correctly!\n");
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0); // socket 생성 TCP 방식 IPv4
    if (sock == -1) 
        error_handling("socket() error", sock); // socket 생성시 error control

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 변환해서 저장
    server_addr.sin_port = htons(atoi(argv[2])); //port num 변환해서 저장

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
        error_handling("connect() error", sock); 

    while (1) {
        read_stat = read(sock, &picture, 1); // 여기서만 선언
        if (read_stat == -1) {
            perror("Client Output; reading data from socket error");
            exit(1);
        } else if (read_stat == 0) {
            break;
        } else {
            if (!strncmp(&gas_check, "1",1)) { // 가스가 감지되면 사진을 찍는다.
                system("raspistill -o image.jpg");
                system("raspistill -vf -o image.jpg");
                sleep(1);
                system("cd project/code");
                system("python3 send_email2.py");
            }
        }
    }

    return 0;
})) { // 가스가 감지되면 사진을 찍는다.
                system("raspistill -o image.jpg");
                system("raspistill -vf -o image.jpg");
                sleep(1);
                system("cd project/code");
                system("python3 send_email2.py");
            }
        }
    }

    return 0;
}
