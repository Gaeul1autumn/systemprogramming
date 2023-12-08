#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

#define PIR_SENSOR_PIN 22  // PIR 센서를 연결한 GPIO 핀

static int spi_fd;
static int sock;

// SPI 통신 설정 함수
static int prepare(int fd)
{
    // SPI 모드, 비트 수, 쓰기 및 읽기 클럭 설정
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1)
    {
        perror("Can't set MODE");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1)
    {
        perror("Can't set number of BITS");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1)
    {
        perror("Can't set write CLOCK");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1)
    {
        perror("Can't set read CLOCK");
        return -1;
    }

    return 0;
}

// SPI 통신을 통한 ADC 값 읽기 함수
int readadc(int fd, uint8_t channel)
{
    uint8_t tx[] = {1, (8 + channel) << 4, 0};  // SPI 전송을 위한 데이터
    uint8_t rx[3] = {0};  // 수신된 데이터를 저장할 배열

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = ARRAY_SIZE(tx),
        .delay_usecs = DELAY,
        .speed_hz = CLOCK,
        .bits_per_word = BITS,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1)
    {
        perror("IO error");
        abort();
    }

    // PIR 센서 값이 감지되면 1, 감지되지 않으면 0 반환
    return (rx[2] > 50) ? 1 : 0;
}

int main(int argc, char *argv[])
{
    char buff[3] = "10";
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

    spi_fd = open(DEVICE, O_RDWR);
    if (spi_fd <= 0)
    {
        perror("Device open error");
        exit(1);
    }

    if (prepare(spi_fd) == -1)
    {
        perror("Device prepare error");
        exit(1);
    }

    while (1)
    {
        int pirValue = readadc(spi_fd, PIR_SENSOR_PIN);

        if (pirValue == 1)
        {
            if (write(sock, &buff[1], 1) == -1)
                perror("Client input; sending data to server error");

            usleep(10000);
        }
    }

    close(spi_fd);

    return 0;
}
