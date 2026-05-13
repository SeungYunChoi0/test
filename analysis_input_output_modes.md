# tc-nn-app: `-i camera` vs `-i rtpm` 동작 차이 분석
**대상 모델:** `-n hand640_qt` (TELECHIPS_NPU_POST_CUSTOM, YOLOv8-pose hand tracking)

---

## 📌 모드 조합 유효성 (NnModeChecker)

```c
// NnAppMain.c L536~549
if(param->inputMode == INPUT_MODE_RTPM && param->outputMode != OUTPUT_MODE_RTPM)
    exit(0);  // ❌ RTPM 입력 시 RTPM 출력만 허용

if(param->inputMode == INPUT_MODE_RTPM && param->outputMode == OUTPUT_MODE_SOCKET)
    exit(0);  // ❌ RTPM + SOCKET 조합 금지
```

> [!IMPORTANT]
> `-i rtpm`은 반드시 `-o rtpm`과 함께 써야 함. `-o display`나 `-o socket`과는 조합 불가.
> 따라서 실제로 가능한 조합은 아래 3가지임.

| 조합 | 유효 여부 |
|------|----------|
| `-i camera -o display` | ✅ 가능 |
| `-i camera -o socket` | ✅ 가능 |
| `-i rtpm -o rtpm` | ✅ 가능 (display처럼 시각화) |
| `-i rtpm -o display` | ❌ **불가 — 앱 강제 종료** |
| `-i rtpm -o socket` | ❌ **불가 — 앱 강제 종료** |

---

## 🗂️ 파라미터 기본값 비교

| 항목 | `-i camera` | `-i rtpm` |
|------|------------|----------|
| `inputFormat` | `IMAGE_FMT_RGBA32` (4ch) | `IMAGE_FMT_RGB24` (3ch) |
| `inputWidth` | `1280` | `1280` (고정) |
| `inputHeight` | `720` | `720` (고정) |
| 입력 소스 | `/dev/video2` (카메라 장치) | RTPM 네트워크 스트림 (AI-G 수신) |
| `inputPath` | `/dev/video2` | N/A (포트 기반) |

---

## 🔄 메인 루프 파이프라인 (공통 8단계)

```
Step 1. NnGetFrame()         ← 입력 모드에 따라 분기
Step 2. NnResizeInputFrame() ← Scaler로 640×640 리사이즈 (NPU 입력용)
Step 3. NnCreateInferenceThread() ← NPU 추론
Step 4. NnResizeOutputFrame() ← 출력 버퍼로 리사이즈
Step 5. NnDrawResult()        ← 출력 버퍼에 bbox/keypoint 그리기
Step 6. NnOutputResultFrame() ← 출력 모드에 따라 분기 (프레임 전송)
Step 7. NnOutputResultData()  ← 출력 모드에 따라 분기 (데이터 전송)
Step 8. NnReleaseFrame()      ← 프레임 버퍼 반환
```

---

## 📋 경우의 수별 상세 동작

---

### ① `-i camera -o display` (LCD 출력)

```
[AI-G 보드]
  카메라(/dev/video2) → RGBA32 프레임 캡처
      ↓ CameraGetBuffer()
  Scaler0: 1280×720 RGBA32 → 640×640 RGB24  ← NPU 입력 리사이즈
      ↓ NPU 추론 (hand640_qt)
  hand_data { num_det, dets[].{x1,y1,x2,y2,score, kpts[21]} } 생성
      ↓ Scaler1: 1280×720 → 800×480 RGB24   ← 출력 버퍼 리사이즈
  NnDrawResult():
    - bbox: (det->x1 * 800/640) 좌표 스케일링 → cvDrawBoxes() [노란색]
    - keypoint: (det->kpts[k].x * 800/640) → cvDrawPoints() [청록색] (conf > 0.5)
    - FPS, CPU/NPU Usage 텍스트 오버레이
      ↓ DisplayShow()
  /dev/overlay LCD (800×480) 직접 표시
      ↓ CameraReleaseBuffer()
```

**핵심 동작:**
- 영상: 카메라 → RGBA32 → Scaler → (NPU입력 + LCD출력) 이중 리사이즈
- 로그: 추론 결과는 로컬 로그만 (`NN_LOG`)
- 시각화: AI-G 보드의 LCD 디스플레이에 bbox + keypoint 실시간 렌더링
- 데이터 전송: **없음** (`NnOutputResultData`에서 LCD 모드는 no-op)

---

### ② `-i camera -o socket` (TCP 소켓 전송)

```
[AI-G 보드]
  카메라(/dev/video2) → RGBA32 프레임 캡처
      ↓ CameraGetBuffer()
  Scaler0: 1280×720 RGBA32 → 640×640 RGB24  ← NPU 입력 리사이즈
      ↓ NPU 추론 (hand640_qt)
  hand_data 생성
      ↓ Scaler1: 1280×720 → 800×480 RGB24   ← 출력 버퍼 리사이즈
      ↓ (Step 5: NnDrawResult() 실행 — 출력 버퍼에 그림 그리지만 전송하지 않음)
  NnOutputResultFrame(): 소켓 모드이므로 프레임 버퍼 출력 없음 (pass-through)

  NnOutputResultData():
    JSON 직렬화 (snprintf, 스택 4096 bytes):
    {
      "info": [syncStamp],
      "type": "hand_pose",
      "hands": [
        {
          "id": 0, "score": 0.92,
          "bbox": [bx, by, bw, bh],   ← 모델(640) → outputWidth/Height 스케일
          "keypoints": [
            {"id": 0, "x": 120, "y": 200, "conf": 0.87},
            ...  (21개)
          ]
        }
      ]
    }
    → TCP send(custom_sockfd, payload, pos, 0)
    → 호스트 IP:8080 으로 전송
      ↓ CameraReleaseBuffer()
```

**핵심 동작:**
- 초기화: `socket()` + `connect()` → 호스트 IP:8080 TCP 연결 (`NnOutputModeInit`)
- 영상: 카메라 → 추론까지는 ① 동일
- 로그: 추론 결과가 JSON으로 직렬화되어 TCP 전송
- 시각화: **AI-G 보드 LCD에는 표시 안 됨** → 호스트가 JSON 수신 후 자체 렌더링
- 성능 모니터 스레드: 생성 안 됨 (RTPM 모드가 아니므로)

> [!NOTE]
> `NnDrawResult()`는 Step 5에서 출력 버퍼에 그리기는 하지만,
> `NnOutputResultFrame()`에서 소켓 모드는 `DisplayShow()` 호출 없이 그냥 통과함.
> 결국 그려진 프레임은 버퍼에만 있고 전송/표시되지 않음.

---

### ③ `-i rtpm -o rtpm` (RTPM 인젝션 모드 + 시각화 전송)

```
[호스트 PC]                          [AI-G 보드]
  웹캠 영상 캡처                         ↕ RTPM 프로토콜 (TCP)
  → Vision API로 AI-G에 스트리밍  →  MessagePopReceiveBuffer()
                                     RGB24 프레임 획득 (syncStamp 포함)
                                         ↓ Scaler0: 1280×720 → 640×640
                                         ↓ NPU 추론 (hand640_qt)
                                     hand_data 생성
                                         ↓ Scaler1: 1280×720 → 1280×720
                                     NnDrawResult():
                                       - bbox/keypoint 를 출력 버퍼에 그림
                                     NnOutputResultFrame():
                                       - input_mode가 RTPM이므로
                                         MessagePushSendBuffer()는 호출 안 됨 (pass)
                                     NnOutputResultData():
                                       - hasCustomResult = true
                                       - RtpmSendHandResultAsJson() 호출
                                         → json-c로 JSON 구성
                                         → Vision_API_SendMessage() 로 전송
                                         → 성능 데이터도 별도 스레드로 전송
                                         (VISION_MSG_EVENT_RESULT_DATA_JSON)
  ← JSON 수신 후 호스트에서 시각화
```

**핵심 동작:**
- 입력: 호스트가 스트리밍하는 RGB24 프레임을 RTPM 소켓으로 수신 (4개 버퍼 풀)
- `syncStamp`: `MessagePopReceiveBuffer()`에서 반환된 타임스탬프 → 프레임 동기화 ID
- 영상 전송: **AI-G → 호스트로 영상 원본 전송 없음** (입력이 호스트→AI-G 방향)
- 데이터 전송: 추론 결과 JSON을 RTPM 메시지 채널로 호스트에게 역전송
- 시각화: 호스트 측 RTPM 수신 앱에서 JSON 파싱 후 웹캠 위에 오버레이
- 성능 스레드: `RtpmCreatePerfmanceDataSendThread()` 시작됨 → CPU/Memory/NPU Usage 주기적 전송

---

## 🏗️ 초기화 컴포넌트 비교

| 컴포넌트 | camera+display | camera+socket | rtpm+rtpm |
|---------|---------------|--------------|----------|
| `CameraCreate()` | ✅ | ✅ | ❌ |
| `DisplayCreate()` | ✅ | ❌ | ❌ |
| `MessageCreate()` | ❌ | ❌ | ✅ |
| TCP `socket()+connect()` | ❌ | ✅ | ❌ |
| Scaler | ✅ | ✅ | ✅ |
| 성능 모니터 스레드 | ❌ | ❌ | ✅ |

---

## 🖼️ Step 1: 프레임 획득 (`NnGetFrame`)

```c
if(inputMode == INPUT_MODE_CAMERA)
    CameraGetBuffer(CameraHandle, &map_base_input, &phy_base_input);
    // → 카메라 DMA 버퍼에서 물리 주소 획득, RGBA32

else if(inputMode == INPUT_MODE_RTPM)
    MessagePopReceiveBuffer(msgHandle, &map_base_input, &phy_base_input, &syncStamp);
    // → RTPM 수신 큐에서 프레임 팝, RGB24, syncStamp에 타임스탬프 저장
```

| | camera | rtpm |
|--|--------|------|
| 데이터 형식 | RGBA32 (4 bytes/px) | RGB24 (3 bytes/px) |
| syncStamp | 갱신 안 됨 (0 유지) | 수신 프레임의 타임스탬프 저장 |
| 버퍼 관리 | 카메라 DMA 버퍼 | RTPM 수신 버퍼 풀 (4개) |
| 실패 시 | retry (CAM_RETRY_CNT) | 블로킹 대기 |

---

## 📐 Step 4: 출력 리사이즈 (`NnResizeOutputFrame`)

```c
if(output_mode == OUTPUT_MODE_RTPM)
{
    if(input_mode == INPUT_MODE_CAMERA)
        MessagePopSendBuffer(msgHandle, ...);  // 전송 버퍼 확보
    // input이 RTPM이면: 버퍼 확보 없음 (입력 버퍼 재활용)
}
// Scaler로 입력→출력 해상도 변환 (모든 모드 공통)
ScalerResize(scalerHandle, outputScalerIdx, scalerSrc, scalerDes);
```

---

## 🎨 Step 5: 시각화 (`NnDrawResult`) — 모든 모드 공통 실행

`hand640_qt` (TELECHIPS_NPU_POST_CUSTOM) 모델 기준:

```c
for (d = 0; d < pNeuralNetwork->hand_data.num_det; d++)
{
    // Bounding Box — 노란색 (255,255,0)
    box.xmin = det->x1 * outputWidth  / HAND_MODEL_INPUT_WIDTH;   // 640→출력폭
    box.ymin = det->y1 * outputHeight / HAND_MODEL_INPUT_HEIGHT;
    box.xmax = det->x2 * outputWidth  / HAND_MODEL_INPUT_WIDTH;
    box.ymax = det->y2 * outputHeight / HAND_MODEL_INPUT_HEIGHT;
    cvDrawBoxes(outputMapBase, &box, 1, ...);

    // Keypoints — 청록색 (0,255,255), conf > 0.5만
    for (k = 0; k < HPD_NUM_KPT; k++)  // 21개 관절
    {
        if (det->kpts[k].conf > 0.5f) {
            kptPoints[kptCount].x = det->kpts[k].x * outputWidth  / 640;
            kptPoints[kptCount].y = det->kpts[k].y * outputHeight / 640;
            kptCount++;
        }
    }
    cvDrawPoints(outputMapBase, ..., kptPoints, kptCount, 3, cyan);
}
cvDrawInfo(..., DRAW_INFO_FPS, NnGetFPS(), ...);   // FPS 표시
cvDrawInfo(..., DRAW_INFO_CPU, cpuUtil, ...);       // CPU 사용률
```

> [!NOTE]
> `NnDrawResult()`는 모든 출력 모드에서 **항상 실행됨**.
> 단, 그 결과(출력 버퍼)가 실제로 보이는지는 Step 6에서 결정됨.

---

## 📤 Step 6: 프레임 출력 (`NnOutputResultFrame`)

| 출력 모드 | 동작 |
|----------|------|
| `display` | `DisplayShow()` → LCD에 픽셀 버퍼 직접 표시 |
| `socket` | **pass-through** (아무것도 안 함, 프레임 미전송) |
| `rtpm` + camera | `MessagePushSendBuffer()` → 영상 프레임을 호스트로 전송 |
| `rtpm` + rtpm | **pass-through** (입력 버퍼 재활용) |

---

## 📨 Step 7: 데이터 출력 (`NnOutputResultData`)

### `-o display` (LCD 모드)
```c
else /* OUTPUT_MODE_LCD mode */ { ; }  // 완전 no-op
```
- 추론 결과 데이터는 어디에도 전송되지 않음

### `-o socket` (소켓 모드)
```c
// snprintf로 JSON 문자열 직접 구성 (json-c 미사용, 스택 버퍼 4096 bytes)
char payload[4096];
pos += snprintf(payload + pos, ..., "{\"info\":[%llu],\"type\":\"hand_pose\",\"hands\":[", syncStamp);
// ... bbox, 21 keypoints 순서대로 직렬화 ...
send(pContext->custom_sockfd, payload, pos, 0);  // TCP 전송
```

### `-o rtpm` (RTPM 모드, custom 모델 시)
```c
// json-c 라이브러리로 JSON 구성 후 Vision API 채널로 전송
RtpmSendHandResultAsJson(msgHandle, syncStamp, pNeuralNetwork, outputWidth, outputHeight);
// → Vision_API_SendMessage(..., VISION_MSG_EVENT_RESULT_DATA_JSON)
// → 추가로 성능 데이터도 전송
RtpmSendInferencePerformanceToRTPM(msgHandle, &inferenceContext);
```

---

## 📊 전체 비교 요약표

| 항목 | `-i camera -o display` | `-i camera -o socket` | `-i rtpm -o rtpm` |
|------|----------------------|----------------------|------------------|
| **입력 소스** | 카메라 장치 `/dev/video2` | 카메라 장치 `/dev/video2` | 호스트 RTPM 스트림 |
| **입력 포맷** | RGBA32 (4ch) | RGBA32 (4ch) | RGB24 (3ch) |
| **syncStamp** | 항상 0 | 항상 0 | 수신 프레임 타임스탬프 |
| **NPU 추론** | 동일 (hand640_qt) | 동일 | 동일 |
| **LCD 시각화** | ✅ AI-G 보드 LCD | ❌ 없음 | ❌ 없음 |
| **프레임 전송** | ❌ 없음 | ❌ 없음 | ✅ 호스트→AI-G (입력) |
| **결과 데이터 전송** | ❌ 없음 | ✅ TCP JSON (port 8080) | ✅ RTPM JSON 채널 |
| **JSON 직렬화** | N/A | snprintf 직접 구성 | json-c 라이브러리 |
| **전송 프로토콜** | N/A | TCP (SOCK_STREAM) | Vision Protocol (TCP) |
| **성능 스레드** | ❌ | ❌ | ✅ (CPU/Mem/NPU 주기 전송) |
| **호스트 시각화** | ❌ | ✅ (호스트 수신 앱 필요) | ✅ (RTPM 수신 앱 필요) |
| **연결 초기화** | 카메라+LCD 드라이버 | 카메라+TCP 소켓 connect | RTPM MessageOpen (recv+send) |

---

## 🔌 소켓 연결 상세 (`-o socket`)

```c
// NnOutputModeInit() — OUTPUT_MODE_SOCKET
pContext->custom_sockfd = socket(AF_INET, SOCK_STREAM, 0);  // TCP
pContext->custom_servaddr.sin_port = htons(8080);
pContext->custom_servaddr.sin_addr.s_addr = inet_addr(pContext->pRtpmIpAdress);
// -t 옵션으로 IP 지정, 기본값: "192.168.0.8"
connect(pContext->custom_sockfd, &custom_servaddr, ...);
```

**JSON 전송 흐름:**
```
AI-G 보드 → TCP 8080 → 호스트 서버 (host_receiver.py 등)
```

---

## ⚠️ 주의 사항

> [!WARNING]
> `-i rtpm -o display` 또는 `-i rtpm -o socket` 조합은 `NnModeChecker()`에서
> `exit(0)` 호출로 **앱이 즉시 종료**됨. 코드에 명시된 제약사항임.

> [!NOTE]
> `-o socket` 모드에서 `NnDrawResult()`는 Step 5에서 여전히 실행되어
> 출력 버퍼에 bbox/keypoint를 그리지만, 그 버퍼는 `DisplayShow()` 되지 않으므로
> 실제로는 **보이지 않음**. LCD 시각화 없이 JSON 데이터만 전송됨.

> [!TIP]
> 호스트 IP 지정은 `-t` 옵션 사용:
> ```bash
> ./tc-nn-app -i camera -o socket -n hand640_qt -t 192.168.0.100
> ./tc-nn-app -i rtpm   -o rtpm   -n hand640_qt -t 192.168.0.100
> ```
