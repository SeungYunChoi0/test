#!/usr/bin/env python3
"""
host_sender.py — tc-nn-app -i host mode / Host PC side
========================================================
Usage:
    python3 host_sender.py --board-ip 192.168.0.100
    python3 host_sender.py --board-ip 192.168.0.100 --width 640 --height 640

동작 흐름:
  1. 호스트 PC에서 웹캠 캡처
  2. 지정 해상도(기본 1280x720)로 리사이즈 후 RGB24 raw bytes → board TCP:9997
  3. 보드에서 NPU 추론 후 JSON 결과 → host TCP:8080 으로 수신
  4. 결과를 웹캠 화면에 핸드 스켈레톤/BBox 오버레이하여 표시

포트:
  BOARD:9997  -- 보드가 프레임 수신용 서버 (board listens)
  HOST:8080   -- 호스트가 추론 결과 수신용 서버 (host listens)
"""

import argparse
import json
import queue
import socket
import sys
import threading
import time

import cv2
import numpy as np

# ---------------------------------------------------------------------------
# Defaults
# ---------------------------------------------------------------------------
DEFAULT_BOARD_IP       = "192.168.0.100"
DEFAULT_SEND_W         = 1280
DEFAULT_SEND_H         = 720
BOARD_FRAME_RECV_PORT  = 9997   # board listens (NnInputModeInit)
HOST_RESULT_RECV_PORT  = 8080   # host listens  (board connects via NnOutputModeInit)
DISPLAY_W              = 1280
DISPLAY_H              = 720

# MediaPipe-style 21-keypoint hand skeleton connections
HAND_CONNECTIONS = [
    (0, 1), (1, 2), (2, 3), (3, 4),          # 엄지
    (0, 5), (5, 6), (6, 7), (7, 8),           # 검지
    (0, 9), (9, 10), (10, 11), (11, 12),      # 중지
    (0, 13), (13, 14), (14, 15), (15, 16),    # 약지
    (0, 17), (17, 18), (18, 19), (19, 20),    # 소지
    (5, 9), (9, 13), (13, 17),                # 손바닥 횡선
]

# ---------------------------------------------------------------------------
# Result receiver thread  (host listens on 8080, board connects)
# ---------------------------------------------------------------------------
result_queue: queue.Queue = queue.Queue(maxsize=2)

def recv_results_thread(listen_port: int) -> None:
    """보드가 전송하는 JSON 추론 결과를 수신하는 백그라운드 스레드."""
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        srv.bind(("0.0.0.0", listen_port))
        srv.listen(1)
        print(f"[HOST] Result receiver listening on port {listen_port} ...")
        srv.settimeout(120)  # 보드 연결 대기 최대 120 s
        conn, addr = srv.accept()
        print(f"[HOST] Board connected for results: {addr}")
        conn.settimeout(5)
        buf = ""
        while True:
            try:
                chunk = conn.recv(4096).decode("utf-8", errors="ignore")
                if not chunk:
                    print("[HOST] Board closed result connection.")
                    break
                buf += chunk
                # 개행 단위로 JSON 파싱
                while "\n" in buf:
                    line, buf = buf.split("\n", 1)
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        parsed = json.loads(line)
                        if result_queue.full():
                            result_queue.get_nowait()
                        result_queue.put_nowait(parsed)
                    except json.JSONDecodeError:
                        pass
            except socket.timeout:
                continue
            except Exception as exc:
                print(f"[HOST][WARN] recv_results_thread: {exc}")
                break
        conn.close()
    except Exception as exc:
        print(f"[HOST][ERROR] recv_results_thread fatal: {exc}")
    finally:
        srv.close()


# ---------------------------------------------------------------------------
# Visualization
# ---------------------------------------------------------------------------
KPT_COLOR  = (0, 100, 255)   # BGR
BONE_COLOR = (0, 220, 50)
BBOX_COLOR = (0, 230, 230)

def draw_hands(frame: np.ndarray, result: dict,
               frame_w: int, frame_h: int,
               board_out_w: int = 800, board_out_h: int = 480) -> np.ndarray:
    """
    JSON 추론 결과를 프레임에 그린다.
    board_out_w/h: 보드가 좌표 스케일링에 사용한 출력 해상도
                   (NnAppMain.c의 outputWidth/outputHeight, 기본 800×480)
    """
    if not result:
        return frame

    hands = result.get("hands", [])
    sx = frame_w / board_out_w   # host 표시 해상도로 재스케일
    sy = frame_h / board_out_h

    for hand in hands:
        score = hand.get("score", 0.0)
        if score < 0.25:
            continue

        bbox = hand.get("bbox", [])
        kpts = hand.get("keypoints", [])

        # BBox 그리기
        if len(bbox) == 4:
            bx = int(bbox[0] * sx)
            by = int(bbox[1] * sy)
            bw = int(bbox[2] * sx)
            bh = int(bbox[3] * sy)
            cv2.rectangle(frame, (bx, by), (bx + bw, by + bh), BBOX_COLOR, 2)
            label = f"hand {score:.2f}"
            cv2.putText(frame, label, (bx, max(by - 6, 14)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.55, BBOX_COLOR, 2, cv2.LINE_AA)

        # keypoint dict 구성
        kpt_dict: dict = {}
        for kp in kpts:
            kid  = kp["id"]
            kx   = int(kp["x"] * sx)
            ky   = int(kp["y"] * sy)
            conf = kp["conf"]
            kpt_dict[kid] = (kx, ky, conf)

        # 스켈레톤 선 그리기
        for (a, b) in HAND_CONNECTIONS:
            if a in kpt_dict and b in kpt_dict:
                xa, ya, ca = kpt_dict[a]
                xb, yb, cb = kpt_dict[b]
                if ca > 0.3 and cb > 0.3:
                    cv2.line(frame, (xa, ya), (xb, yb), BONE_COLOR, 2, cv2.LINE_AA)

        # 키포인트 점 그리기
        for kid, (kx, ky, conf) in kpt_dict.items():
            if conf > 0.3:
                color = (0, 255, 0) if kid == 0 else KPT_COLOR
                cv2.circle(frame, (kx, ky), 4, color, -1, cv2.LINE_AA)

    return frame


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="tc-nn-app -i host mode: host PC webcam sender + result visualizer"
    )
    p.add_argument("--board-ip",   default=DEFAULT_BOARD_IP,
                   help=f"AI-G board IP address (default: {DEFAULT_BOARD_IP})")
    p.add_argument("--width",      type=int, default=DEFAULT_SEND_W,
                   help=f"Frame width  sent to board (default: {DEFAULT_SEND_W})")
    p.add_argument("--height",     type=int, default=DEFAULT_SEND_H,
                   help=f"Frame height sent to board (default: {DEFAULT_SEND_H})")
    p.add_argument("--cam-id",     type=int, default=0,
                   help="Webcam device index (default: 0)")
    p.add_argument("--result-port", type=int, default=HOST_RESULT_RECV_PORT,
                   help=f"Port to receive inference results (default: {HOST_RESULT_RECV_PORT})")
    p.add_argument("--board-out-w", type=int, default=800,
                   help="Board output width used for coordinate scaling (default: 800)")
    p.add_argument("--board-out-h", type=int, default=480,
                   help="Board output height used for coordinate scaling (default: 480)")
    return p.parse_args()


def main() -> None:
    args = parse_args()

    BOARD_IP    = args.board_ip
    SEND_W      = args.width
    SEND_H      = args.height
    FRAME_BYTES = SEND_W * SEND_H * 3  # RGB24

    print("=" * 60)
    print("  tc-nn-app  -i host  /  Host Sender")
    print("=" * 60)
    print(f"  Board IP          : {BOARD_IP}")
    print(f"  Frame send port   : {BOARD_FRAME_RECV_PORT}  (board listens)")
    print(f"  Result recv port  : {args.result_port}  (host listens)")
    print(f"  Send resolution   : {SEND_W} x {SEND_H}  RGB24")
    print(f"  Board output res  : {args.board_out_w} x {args.board_out_h}")
    print("=" * 60)

    # ------------------------------------------------------------------
    # Step 1: 결과 수신 스레드 먼저 시작 (보드 NnOutputModeInit 전에 준비)
    # ------------------------------------------------------------------
    t_recv = threading.Thread(
        target=recv_results_thread,
        args=(args.result_port,),
        daemon=True,
    )
    t_recv.start()

    # ------------------------------------------------------------------
    # Step 2: 웹캠 열기
    # ------------------------------------------------------------------
    cap = cv2.VideoCapture(args.cam_id)
    if not cap.isOpened():
        print(f"[HOST][ERROR] Cannot open webcam id={args.cam_id}")
        sys.exit(1)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  SEND_W)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, SEND_H)
    cap.set(cv2.CAP_PROP_FPS, 30)
    print(f"[HOST] Webcam opened: cam_id={args.cam_id}")

    # ------------------------------------------------------------------
    # Step 3: 보드 9997 포트에 TCP 접속 (board NnInputModeInit accept 트리거)
    # ------------------------------------------------------------------
    print(f"[HOST] Connecting to board {BOARD_IP}:{BOARD_FRAME_RECV_PORT} ...")
    send_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connected = False
    for attempt in range(30):
        try:
            send_sock.connect((BOARD_IP, BOARD_FRAME_RECV_PORT))
            connected = True
            break
        except (ConnectionRefusedError, OSError):
            print(f"[HOST] Waiting for board... ({attempt + 1}/30)")
            time.sleep(2)
    if not connected:
        print("[HOST][ERROR] Could not connect to board. Check board IP / port.")
        cap.release()
        sys.exit(1)
    print(f"[HOST] Connected to board for frame sending.")

    # ------------------------------------------------------------------
    # Step 4: 메인 루프
    # ------------------------------------------------------------------
    last_result: dict = {}
    frame_cnt   = 0
    t_start     = time.time()
    fps_display = 0.0

    print("[HOST] Streaming... press 'q' to quit.")
    try:
        while True:
            t0 = time.perf_counter()

            ret, frame = cap.read()
            if not ret:
                print("[HOST][WARN] Webcam read failed.")
                time.sleep(0.01)
                continue

            # BGR → RGB, resize
            frame_rgb = cv2.cvtColor(
                cv2.resize(frame, (SEND_W, SEND_H)), cv2.COLOR_BGR2RGB
            )
            raw_bytes = frame_rgb.tobytes()  # SEND_W * SEND_H * 3

            # 프레임 전송 (TCP send)
            try:
                send_sock.sendall(raw_bytes)
            except BrokenPipeError:
                print("[HOST][ERROR] Board disconnected (send).")
                break

            # 최신 추론 결과 가져오기 (non-blocking)
            try:
                last_result = result_queue.get_nowait()
            except queue.Empty:
                pass

            # 시각화 (BGR frame 위에 결과 오버레이)
            display = cv2.resize(frame, (DISPLAY_W, DISPLAY_H))
            draw_hands(display, last_result, DISPLAY_W, DISPLAY_H,
                       args.board_out_w, args.board_out_h)

            # HUD 텍스트
            frame_cnt += 1
            elapsed_total = time.time() - t_start
            if elapsed_total > 0:
                fps_display = frame_cnt / elapsed_total
            cv2.putText(display, f"FPS: {fps_display:.1f}", (10, 32),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255, 220, 0), 2, cv2.LINE_AA)
            cv2.putText(display,
                        f"Board: {BOARD_IP}:{BOARD_FRAME_RECV_PORT}  "
                        f"Send: {SEND_W}x{SEND_H}",
                        (10, 62), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                        (200, 200, 200), 1, cv2.LINE_AA)
            mode_txt = "MODE: -i host -o socket"
            cv2.putText(display, mode_txt, (10, DISPLAY_H - 12),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (100, 255, 100), 1, cv2.LINE_AA)

            cv2.imshow("tc-nn-app  |  -i host -o socket", display)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                print("[HOST] 'q' pressed — exiting.")
                break

            # 전송 속도 조절 (≈30 fps)
            elapsed_frame = time.perf_counter() - t0
            sleep_t = (1.0 / 30.0) - elapsed_frame
            if sleep_t > 0:
                time.sleep(sleep_t)

    except KeyboardInterrupt:
        print("\n[HOST] Keyboard interrupt.")
    finally:
        cap.release()
        send_sock.close()
        cv2.destroyAllWindows()
        print("[HOST] Done.")


if __name__ == "__main__":
    main()
