import os
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage

# 보내는 사람 이메일 계정 설정
sender_email = "your_email@gmail.com"
sender_password = "your_password"

# 받는 사람 이메일 주소
recipient_email = "recipient_email@example.com"

# 이메일 서버 설정 (Gmail 예제)
smtp_server = "smtp.gmail.com"
smtp_port = 587

# 이미지 파일이 있는 디렉토리 설정
image_dir = "path/to/your/image/directory"

# 디렉토리에서 모든 이미지 파일 가져오기
image_files = [f for f in os.listdir(image_dir) if f.lower().endswith(('.png', '.jpg', '.jpeg'))]

# 최근 수정된 이미지 파일 찾기
latest_image = max(image_files, key=lambda x: os.path.getmtime(os.path.join(image_dir, x)))

# 최근 이미지 파일 경로
image_path = os.path.join(image_dir, latest_image)

# 이메일 객체 생성
msg = MIMEMultipart()
msg['From'] = sender_email
msg['To'] = recipient_email
msg['Subject'] = "이메일 제목"

# 이메일 본문 추가
body = "이메일 본문 내용"
msg.attach(MIMEText(body, 'plain'))

# 최근 이미지 첨부
with open(image_path, "rb") as image_file:
    image = MIMEImage(image_file.read())
    msg.attach(image)

# SMTP 서버 연결 및 이메일 전송
with smtplib.SMTP(smtp_server, smtp_port) as server:
    server.starttls()
    server.login(sender_email, sender_password)
    server.sendmail(sender_email, recipient_email, msg.as_string())

print(f"최근 이미지 '{latest_image}'를 이메일로 전송했습니다.")
