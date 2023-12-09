/* 
이 부분 main 함수에 추가해주세요. 서버에서 1를 받으면 가스가 발생했다는 뜻으로 카메라를 작동시키는 로직입니다.
int sock; // 전역

int main(int argc, char*argv[]){
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
                사진 찍기 
                sleep(1);
                system("cd 사진 있는 경로로 이동");
                system("python3 send_email.py");
            }
        }

 }
*/






#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#define SENDER_EMAIL "user1@gmail.com"
#define SENDER_PASSWORD "password"
#define RECIPIENT_EMAIL "user2@gmail.com"
#define SUBJECT "이메일 제목"
#define BODY "이메일 본문 내용"

int main() {
    // 1. 사진을 찍어 파일로 저장
    system("raspistill -o image.jpg");

    CURL *curl;
    CURLcode res = CURLE_OK;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl){
        // 2. 이메일 서버 설정 (Gmail 예제)
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(curl, CURLOPT_USERNAME, SENDER_EMAIL);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, SENDER_PASSWORD);

        // 3. 이메일 내용 설정
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, SENDER_EMAIL);
        struct curl_slist *recipients = NULL;
        recipients = curl_slist_append(recipients, RECIPIENT_EMAIL);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // 4. 이메일 헤더 설정
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: multipart/mixed;");
        headers = curl_slist_append(headers, "charset=UTF-8;");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 5. 이메일 본문 설정
        curl_mime *mime;
        curl_mimepart *part;
        mime = curl_mime_init(curl);

        part = curl_mime_addpart(mime);
        curl_mime_data(part, BODY, CURL_ZERO_TERMINATED);

        // 6. 이미지 파일 첨부
        part = curl_mime_addpart(mime);
        curl_mime_filedata(part, "image.jpg");  // 이미지 파일의 경로

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        // 7. CURL 작업 수행
        res = curl_easy_perform(curl);

        // 8. 에러 처리
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // 9. 메모리 및 리소스 정리
        curl_slist_free_all(headers);
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
        curl_mime_free(mime);

        // 10. 이미지 파일 삭제
        remove("image.jpg");

        // curl_global_cleanup();  // 이 부분을 사용하려면 다른 curl 사용 부분이 없어야 합니다.
    }

    return (int)res;
}
