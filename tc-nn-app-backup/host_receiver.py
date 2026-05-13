import socket
import json

def start_tcp_server(host='0.0.0.0', port=8080):
    # TCP 소켓 생성
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 포트 재사용 옵션 설정 (서버 재시작 시 Address already in use 에러 방지)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    server_socket.bind((host, port))
    server_socket.listen(1)
    print(f"[*] 호스트 서버가 {port} 포트에서 대기 중입니다...")
    print(f"[*] 보드에서 'tc-nn-app -i camera -o socket -t <호스트IP>' 를 실행해주세요.\n")

    while True:
        client_socket, addr = server_socket.accept()
        print(f"[+] 보드 연결됨: {addr[0]}:{addr[1]}")
        
        # 파일 객체처럼 다루기 위해 makefile 사용 (줄바꿈 '\n' 기준으로 쉽게 읽기 위함)
        client_file = client_socket.makefile('r')
        
        try:
            while True:
                # 보드에서 '\n'을 끝으로 보내는 JSON 한 줄을 읽음
                line = client_file.readline()
                if not line:
                    print("[-] 보드와의 연결이 끊어졌습니다.")
                    break
                
                line = line.strip()
                if line:
                    try:
                        # JSON 파싱
                        data = json.loads(line)
                        print("\n[수신된 추론 결과]")
                        print(json.dumps(data, indent=2, ensure_ascii=False))
                        
                        # -- 이 부분에 수신된 키포인트 데이터를 활용하는 로직을 추가하세요 --
                        # 예: data['hands'][0]['keypoints'] 를 활용해 화면에 점을 찍거나 로봇 암 제어 등
                        
                    except json.JSONDecodeError:
                        print(f"[!] JSON 파싱 에러. 원본 데이터: {line}")
                        
        except ConnectionResetError:
            print("[-] 보드에서 강제로 연결을 종료했습니다.")
        finally:
            client_file.close()
            client_socket.close()

if __name__ == "__main__":
    start_tcp_server(host='0.0.0.0', port=8080)
