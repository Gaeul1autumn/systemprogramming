#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

int server_sock; 
pthread_t th[3];
int gas_detection = 0;
int motion_detection = 0;
FILE* fp;

struct connecting_socket{ // connection 요청하는 client와 연결하는 socket 생성시 사용하는 구조체
    struct sockaddr_in socket_addr ;
    socklen_t socket_addr_size;
    int sock;
}cs[3];

void* read_gasvalue(void * socket){
    char gas_check;
    struct connecting_socket* sk = (struct connecting_socket*)socket;
    ssize_t read_stat;
    while(1){
        read_stat = read(sk->sock, &gas_check, 1);
        if(read_stat == -1) {
            perror("Client Output; reading data from socket error");
            exit(1);
        }
        else if(read_stat == 0) break;
        else{
            printf("%c\n",gas_check);
            if(!strncmp(&gas_check,"1",1)){ //가스 발생
                gas_detection = 1;
                sleep(1);
            }else {
                gas_detection = 0;
                sleep(1);
            }
        }
    }

    close(sk->sock);
    printf("connection with client is closed!\n");
    exit(0); 
}

void* camera(void * socket){
    char buff[3] = "10";
    struct connecting_socket* sk = (struct connecting_socket*)socket;
    while(1){
        if(gas_detection && motion_detection){
            if(write(sk->sock,&buff[0],1)== -1) perror(" write error");
            system("espeak ""흡연이 감지되었습니다: 이곳은 금연구역 입니다"" -s 160 -p 95 -v ko+f3");
            siren();
        }else{
            if(write(sk->sock,&buff[1],1)== -1) perror(" write error");
        }
    }
}

void* motion(void * socket){
    char motion_check;
    ssize_t read_stat;
    struct connecting_socket* sk = (struct connecting_socket*)socket;
    while(1){
        read_stat = read(sk->sock, &motion_check, 1);
        if(read_stat == -1) {
            perror("Client Output; reading data from socket error");
            exit(1);
        }
        else if(read_stat == 0) break;
        else{
            if(!strncmp(&motion_check,"1",1)){ //가스 발생
                motion_detection = 1;
                sleep(1);
            }else {
                motion_detection = 0;
                sleep(1);
            }
        }
    }

    close(sk->sock);
    printf("connection with client is closed!\n");
    exit(0); 
}


void error_handling(char *message, int sock) {//socket error날 시 handle
    close(sock); // socket 닫기
    perror(message); // error message 출력
    fputc('\n', stderr);
    exit(1); // 종료
}

void sig_handler(int signo){ // ctrl + C 로 server  강제 종료할 때 handling
    printf("\n");
    close(server_sock); // 서버 socket 닫기
    fclose(fp);
    int fd = open("/sys/class/pwm/pwmchip0/unexport",O_WRONLY);
    write(fd,"0",1);
    // write(fd,"1",1);
    close(fd);
    exit(0); // 종료 
}

/* void set_gpio(){
    int fd = open("/sys/class/gpio/export", O_WRONLY);
      if (fd == -1) {
        perror("Error exporting GPIO");
        return -1;
    }

    write(fd,"22",3);
    write(fd,"27",3);
    close(fd);

    sleep(1);

    fd = open("/sys/class/gpio/gpio22/direction", O_WRONLY);
    if (fd == -1) {
        perror("Error set direction gpio");
        return -1;
    }
    write(fd,"out",3);
    close(fd);

    fd = open("/sys/class/gpio/gpio27/direction", O_WRONLY);
    if (fd == -1) {
        perror("Error set direction gpio");
        return -1;
    }

    write(fd,"out",3);
    close(fd);

} */

void pwm(){
    int fd = open("/sys/class/pwm/pwmchip0/export",O_WRONLY);
    write(fd,"0",1);
    close(fd);
    sleep(1);

    fd = open("/sys/class/pwm/pwmchip0/pwm0/period", O_WRONLY);
    write(fd,"1000000",6);
    close(fd);

    fd=open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
    write(fd,"0",1);
    close(fd);

    fd= open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY);
    write(fd,"1",1);
    close(fd);

    fp=fopen("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", "w");

    /* fd = open("/sys/class/pwm/pwmchip0/export",O_WRONLY);
    write(fd,"1",1);
    close(fd);
    sleep(1);

    fd = open("/sys/class/pwm/pwmchip0/pwm1/period", O_WRONLY);
    write(fd,"1000000",6);
    close(fd);

    fd=open("/sys/class/pwm/pwmchip0/pwm1/duty_cycle", O_WRONLY);
    write(fd,"0",1);
    close(fd);

    fd= open("/sys/class/pwm/pwmchip0/pwm1/enable", O_WRONLY);
    write(fd,"1",1);
    close(fd);

    fp=fopen("/sys/class/pwm/pwmchip0/pwm1/duty_cycle", "w");
 */
}

void siren(){
    time_t start = time(NULL);
    while(time(NULL)-start <= 5){
         for (int i = 0; i < 1000; i++){
            fprintf(fp,"%d",i*1000);
        }
        for(int i = 1000; i >0; i--){
            fprintf(fp, "%d",i*1000);
        }
    }
     fprintf(fp, "%d",0);
  
}

int main( int argc, char*argv[]){
    struct sockaddr_in server_addr; 
    int i = 0;
    signal(SIGINT, (void*)sig_handler);

    if(argc !=2 ){ // server 프로그램 실행시 port num을 인자로 입력하지 않았을 때
        printf("Please deliver Port num as argument Correctly!\n");
        exit(1);
    }
    
    server_sock = socket(PF_INET, SOCK_STREAM, 0); // ipv4 서버 socket 생성 
    if(server_sock== -1) error_handling("socket() error", server_sock);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // ipv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버 Ip 자동 대입
    server_addr.sin_port = htons(atoi(argv[1])); // 인자로 받은 port num 저장 

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)  // 서버 socket에 ip 포트넘버 지정 
        error_handling("bind() error", server_sock);

    if (listen(server_sock, 5) == -1) error_handling("listen() error", server_sock); // client connection 요청 듣기 


    while(1) {
        if(i==4) break;
        cs[i].socket_addr_size = sizeof(cs[i].socket_addr);
        cs[i].sock = accept(server_sock, (struct sockaddr *)&(cs[i].socket_addr), &(cs[i].socket_addr_size)); // client connection 요청 받아들이고, connecting socket 생성
        
        if (cs[i].sock == -1) { // accept 함수 error control 
            perror("accept() error");
            close(cs[i].sock); // socket 닫고
            continue; // 다시 accept할 수 있도록 continue
        }

        // client와의 connection이 성립되어 join 하게되었음을 출력 
        printf("Client join fd[%d], ip:[%s], port:[%d]\n",cs[i].sock,inet_ntoa(cs[i].socket_addr.sin_addr), ntohs(cs[i].socket_addr.sin_port));

        if(!(strncmp("192.168.112",inet_ntoa(cs[i].socket_addr.sin_addr),11))){ // 모션 FIXME: ip 수정 필수 
            if(pthread_create(&th[i], NULL, motion, (void *)&cs[i] )){ 
                perror("thread creating error");
                close(cs[i].sock); 
                continue;
            }

        }else if(!(strncmp("192.168.111.7",inet_ntoa(cs[i].socket_addr.sin_addr),13))){ // 가스
            if(pthread_create(&th[i], NULL,  read_gasvalue, (void *)&cs[i] )){
                perror("thread creating error");
                close(cs[i].sock); 
                continue; 
            }
            printf("join\n");


        }else if(!(strncmp("192.168.114",inet_ntoa(cs[i].socket_addr.sin_addr),11))){ // 카메라
            if(pthread_create(&th[i], NULL,  camera, (void *)&cs[i] )){ // client와 connection 수락한 후 바로 shell을 열기 위한 thread 생성 
                perror("thread creating error");
                close(cs[i].sock); // 에러 발생시  socket닫고 메모리도 해제
                continue; // socket accept부터 다시 while문 돌기
            }
        }
        
      

        pthread_detach(th[i++]); // 생성한 thread룰 main 에서 분리해서 원격 shell 종료시 자동 자원 반환하도록 

        
    }

    while(1){}

    return 0;


}


/* 
int sock;
int argc, char*argv[]
 struct sockaddr_in server_addr; 
if (argc != 3) { // client 프로그램 실행 시 인자에 IP랑 port num을 제대로 주지 않았을 때 
        printf("Please deliver IP & Port num as arguments Correctly!\n");
        exit(1);
    }

sock = socket(PF_INET, SOCK_STREAM, 0); // socket 생성 TCP 방식 IPv4
if (sock== -1) error_handling("socket() error", sock); // socket 생성시 error control
    
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET; //IPv4
server_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 변환해서 저장
server_addr.sin_port = htons(atoi(argv[2])); //port num 변환해서 저장

if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) error_handling("connect() error", sock);

}

 */

/* 
char buff[3]="10"
if(가스 && 일산화 모두 감지){
    if(write(sock, &buff[0], 1) == -1) perror("Client input; sending data to server error");
}else{
    if(write(sock, &buff[1], 1) == -1) perror("Client input; sending data to server error");
 */


/* 

char buff[3]="10"
if(모션 감지 됨){
    if(write(sock, &buff[0], 1) == -1) perror("Client input; sending data to server error");
}else{
    if(write(sock, &buff[1], 1) == -1) perror("Client input; sending data to server error");
 */


/* 
ssize_t read_stat;
char picture;
    while(1){
        read_stat = read(sock, &picture, 1);
        if(read_stat == -1) {
            perror("Client Output; reading data from socket error");
            exit(1);
        }
        else if(read_stat == 0) break;
        else{
            if(!strncmp(&gas_check,"1",1)){ // 사진 찍음
                사진 찍기 함수
                sleep(1);
                system("cd 사진 있는 경로로 이동");
                system("python3 email.py");
            }
        }
 */
