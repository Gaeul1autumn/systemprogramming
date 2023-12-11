#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int sock;

#define PIR_PIN 17 // PIR 센서가 연결된 GPIO 핀 번호

// GPIO 핀 초기화 함수
void init_gpio() {
    int fd;

    // GPIO 핀을 export
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        perror("Error exporting GPIO");
        exit(1);
    }
    write(fd, "17", 2);
    close(fd);

    // 방향 설정 (in으로 설정)
    fd = open("/sys/class/gpio/gpio17/direction", O_WRONLY);
    if (fd == -1) {
        perror("Error setting direction GPIO");
        exit(1);
    }
    write(fd, "in", 2);
    close(fd);
}

// GPIO 핀 읽기 함수
int read_gpio() {
    int fd;
    char buffer[2];

    // 값 읽기
    fd = open("/sys/class/gpio/gpio17/value", O_RDONLY);
    if (fd == -1) {
        perror("Error reading GPIO value");
        exit(1);
    }
    read(fd, buffer, sizeof(buffer));
    close(fd);

    return atoi(buffer);
}

int main(int argc, char *argv[]) {
    printf("PIR Sensor Test\n");
    

    // GPIO 핀 초기화
    init_gpio();
    struct sockaddr_in server_addr;

    if (argc != 3)
    {
        printf("Please deliver IP & Port num as arguments correctly!\n");
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }
    printf("join success!\n");

    FILE* fp = fopen("/sys/class/gpio/gpio17/value","r");
    sleep(1);

    while(1){
        int motion_check = read_gpio();

        if (motion_check) {
            printf("Motion detected!\n");
            if(write(sock, "1", 1)==-1){
                perror("Client sending data error"); 
                return -1;
                 }
           
        } else {
            printf("No motion.\n");
            if(write(sock, "0", 1)==-1){
            perror("Client sending data error"); 
            return -1;
            }
        }
        sleep(1); // 1초 간격으로 센서 상태를 확인


    }
}
