#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;
int sock;

void error_handling(char *message, int sock)
{                    // socket error날 시 handle
    close(sock);     // socket 닫기
    perror(message); // error message 출력
    fputc('\n', stderr);
    exit(1); // 종료
}

static int prepare(int fd)
{
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

uint8_t control_bits_differential(uint8_t channel)
{
  return (channel & 7) << 4;
}

uint8_t control_bits(uint8_t channel)
{
  return 0x8 | control_bits_differential(channel);
}

int readadc(int fd, uint8_t channel)
{
  uint8_t tx[] = {1, control_bits(channel), 0};
  uint8_t rx[3];

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
    perror("IO Error");
    abort();
  }

  return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}

int main(int argc, char *argv[])
{
  
  char val[3] = "10";
  struct sockaddr_in server_addr;
  printf("hello\n");

  if (argc != 3)
  {
    printf("Please deliver IP & Port num as argemnts Correctly!");
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
    error_handling("connect() error", sock); // 서버연결
    
  printf("join\n");

  int fd = open(DEVICE, O_RDWR);
  if (fd <= 0)
  {
    perror("Device open error");
    return -1;
  }

  if (prepare(fd) == -1)
  {
    perror("Device prepare error");
    return -1;
  }

  while (1)
  {
    printf("value: %d , %d\n", readadc(fd, 0), readadc(fd, 1)); // 가스센서 채널:0 일산화탄소 채널:1
    if (readadc(fd, 0) > 300 && readadc(fd, 1) > 300)
    { // 아날로그 신호 읽기
      if (write(sock, &val[0], 1) == -1)
      {
        perror("Client sending data error"); // 가스발생하면 서버에 1보내기
        return -1;
      }
    }
    else if (write(sock, &val[1], 1) == -1)
    {
      perror("Client sending data error"); // 발생하지 않으면 0보내기
      return -1;
    }
    sleep(1);
  }
}
