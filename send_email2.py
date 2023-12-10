//version 2

import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage

# 보내는 사람 이메일 계정 설정
sender_email = "sarah123456w@naver.com"
sender_password = ""

# 받는 사람 이메일 주소
recipient_email = "day305@naver.com"

# 이메일 서버 설정 (Gmail 예제)
smtp_server = "smtp.naver.com"
smtp_port = 587

# 이메일 객체 생성
msg = MIMEMultipart()
msg['From'] = sender_email
msg['To'] = recipient_email
msg['Subject'] = "이메일 제목"

# 이메일 본문 추가
body = "이메일 본문 내용"
msg.attach(MIMEText(body, 'plain'))

# 이미지 첨부
with open("/home/pi/cat.jpg", "rb") as image_file:
    image = MIMEImage(image_file.read())
    msg.attach(image)

# SMTP 서버 연결 및 이메일 전송
with smtplib.SMTP(smtp_server, smtp_port) as server:
    server.starttls()
    server.login(sender_email, sender_password)
    server.sendmail(sender_email, recipient_email, msg.as_string())


