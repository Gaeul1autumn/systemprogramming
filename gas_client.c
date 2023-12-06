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

#define GAS_SENSOR_PIN_1 20
#define GAS_SENSOR_PIN_2 21

// 서버 정보
#define SERVER_IP "127.0.0.1" // 서버의 IP 주소
#define SERVER_PORT 8080      // 서버의 포트 번호

static int spi_fd;
static pthread_t thread_id;

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
    perror("IO error");
    abort();
  }

  // 가스가 대기 상태보다 높은 값이 감지되면 1, 그렇지 않으면 0 반환
  return (rx[2] > 50) ? 1 : 0;
}

void *send_sensor_values(void *arg)
{
  int server_fd, new_socket;
  struct sockaddr_in server, client;
  int opt = 1;
  int addrlen = sizeof(client);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(SERVER_IP);
  server.sin_port = htons(SERVER_PORT);


  while (1)
  {
    // 클라이언트 연결 수락
    if ((new_socket = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&addrlen)) < 0)
    {
      perror("accept() error");
      exit(EXIT_FAILURE);
    }

    // 가스 센서 값 읽어와서 전송
    int gasValue1 = readadc(spi_fd, GAS_SENSOR_PIN_1);
    int gasValue2 = readadc(spi_fd, GAS_SENSOR_PIN_2);

    // 클라이언트에게 메시지 전송
    char message[50];
    sprintf(message, "%d %d", gasValue1, gasValue2);
    send(new_socket, message, strlen(message), 0);

    // 연결 종료
    close(new_socket);
  }
}

int sock;

int main(int argc, char *argv[])
{
  struct sockaddr_in server_addr;
  if (argc != 3)
  { // client 프로그램 실행 시 인자에 IP랑 port num을 제대로 주지 않았을 때
    printf("Please deliver IP & Port num as arguments Correctly!\n");
    exit(1);
  }

  sock = socket(PF_INET, SOCK_STREAM, 0); // socket 생성 TCP 방식 IPv4
  if (sock == -1)
    error_handling("socket() error", sock); // socket 생성시 error control

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;                 // IPv4
  server_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 변환해서 저장
  server_addr.sin_port = htons(atoi(argv[2]));      // port num 변환해서 저장

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    error_handling("connect() error", sock);



// SPI 장치 열기
spi_fd = open(DEVICE, O_RDWR);
if (spi_fd <= 0)
{
  perror("Device open error");
  return -1;
}

// SPI 설정 준비
if (prepare(spi_fd) == -1)
{
  perror("Device prepare error");
  return -1;
}

// 스레드 시작
if (pthread_create(&thread_id, NULL, send_sensor_values, NULL) != 0)
{
  perror("thread creation failed");
  return -1;
}

// 메인 스레드에서 가스 센서 값을 읽고 출력
while (1)
{
  int gasValue1 = readadc(spi_fd, GAS_SENSOR_PIN_1);
  int gasValue2 = readadc(spi_fd, GAS_SENSOR_PIN_2);

  //fixme
  char buff[3] = "10";
  if (gasValue1 == 1 && gasValue2 == 1)
  {
    if (write(sock, &buff[0], 1) == -1)
      perror("Client input; sending data to server error");
  }
  else
  {
    if (write(sock, &buff[1], 1) == -1)
      perror("Client input; sending data to server error");

    usleep(10000);
  }

  // 스레드 종료 대기
  pthread_join(thread_id, NULL);

  // SPI 장치 닫기
  close(spi_fd);

  return 0;
}
