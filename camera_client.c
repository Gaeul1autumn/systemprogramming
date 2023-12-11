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

int sock;

void error_handling(const char *message, int sock) {
    close(sock);
    perror(message);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[]) {
    struct sockaddr_in server_addr; 
    char picture;
    ssize_t read_stat;

    if (argc != 3) { 
        printf("Please deliver IP & Port num as arguments Correctly!\n");
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) 
        error_handling("socket() error", sock);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
        error_handling("connect() error", sock); 

    while (1) {
        read_stat = read(sock, &picture, 1);
        if (read_stat == -1) {
            perror("Client Output; reading data from socket error");
            exit(1);
        } else if (read_stat == 0) {
            break;
        } else {
            if (!strncmp(&picture, "1",1)) {
                system("raspistill -o image.jpg");
                sleep(1);
                system("python3 send_email2.py");
            }
        }
    }

    return 0;
}
