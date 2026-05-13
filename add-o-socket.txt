

## 작업 기본 경로
모든 작업은 다음 경로 내에서 수행
`/Volumes/Yun_ssd/Telechips_Project/tc-nn-toolkit/git_test/test/tc-nn-app/`

---

##  파일별 수정 내역 상세

### 1. `include/NnType.h`
*   **수정된 구조체(Enum):** `output_data_type_t`
*   **수정 내용:** 기존의 `OUTPUT_MODE_DISPLAY`, `OUTPUT_MODE_RTPM`, `OUTPUT_MODE_FILE` 외에 소켓 통신 상태를 식별하기 위해 **`OUTPUT_MODE_SOCKET`** 열거형(Enum) 값을 새롭게 추가했습니다.

### 2. `src/NnAppMain.h`
*   **수정된 구조체:** `app_context_t`
*   **수정 내용:**
    *   TCP/IP 소켓 통신 및 JSON 처리를 위해 `<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`, `<json-c/json.h>` 표준 라이브러리를 `include` 하였습니다.
    *   생성된 소켓의 파일 디스크립터 번호를 전역 문맥에서 유지하기 위해 `int socketFd;` 변수를 추가했습니다.

### 3. `src/NnAppMain.c`
아래의 함수들이 수정되었습니다.

#### ① `NnparseArgs` (커맨드라인 파싱)
*   **수정 내용:** 커맨드라인에서 `-o` 옵션을 파싱하는 `case 'o':` 구문 내부에 `else if(strcmp(optarg, "socket") == 0)` 분기를 추가했습니다. 파싱 성공 시 `param->outputMode = OUTPUT_MODE_SOCKET;` 으로 상태를 저장하도록 처리했습니다.

#### ② `NnprintUsage` (사용법 출력)
*   **수정 내용:** `--help` 등을 통해 출력되는 도움말 텍스트 중 `[Common]Output Mode` 항목의 설명란(`option: display, rtpm, file`) 끝에 `, socket` 텍스트를 추가하여 사용자가 옵션의 존재를 알 수 있도록 수정했습니다. (전역 배열 `opt_str.outputModeStr` 에도 `"socket"` 추가)

#### ③ `NnInitAppContext` (앱 문맥 초기화)
*   **수정 내용:** 초기 문맥 설정 시, 모드가 `OUTPUT_MODE_SOCKET`일 경우 사용할 포트 번호를 `pContext->messagePort = 8080;`으로 고정 세팅하고, `-t` 옵션으로 입력받은 타겟 IP 주소를 복사한 뒤, 초기 상태를 위해 `pContext->socketFd = -1;`로 세팅하는 로직을 추가했습니다.

#### ④ `NnOutputModeInit` (출력 모드 초기화 - 연결 수행)
*   **수정 내용:** 애플리케이션 시작 단계에서 실제로 타겟과 TCP 연결을 맺는 핵심 로직을 추가했습니다. `socket(AF_INET, SOCK_STREAM, 0)` 함수를 호출하여 소켓을 생성하고, 입력받은 IP 주소(`inet_pton`)와 8080 포트(`htons`)를 향해 능동적으로 클라이언트로서 `connect()` 하도록 구현했습니다.

#### ⑤ `NnOutputResultData` (JSON 데이터 전송)
*   **수정 내용:** 모델 추론이 완료된 후 후처리 데이터를 네트워크로 보내는 통신 로직입니다. 
    *   `output_mode == OUTPUT_MODE_SOCKET` 조건일 때, `custom_postproc.c`에서 생성된 손가락 21개의 키포인트(x, y, conf)와 Bbox 수치를 읽어옵니다.
    *   `json-c` 라이브러리를 사용하여 RTPM 포맷(`hands` 배열 내에 `id`, `bbox`, `keypoints`를 포함하는 구조)과 완전히 동일한 형태의 JSON 객체를 생성합니다.
    *   완성된 JSON을 문자열 변환하여 TCP `send()` 시스템 콜을 통해 타겟 서버로 전송하고, 끝에 `\n`를 덧붙여 전송이 끝났음을 알립니다.

#### ⑥ `NnOutputResultFrame` (LCD 동시 시각화 지원)
*   **수정 내용:** 데이터 송신과 별개로 보드 화면에 현재 카메라 상태와 추론된 스켈레톤 선을 띄우기 위해, `switch (output_mode)`에 `case OUTPUT_MODE_SOCKET:`을 새롭게 추가하고 내부에서 `DisplayShow()` API를 호출하도록 수정했습니다.

#### ⑦ `NnOutputModeDeinit` (자원 해제)
*   **수정 내용:** 애플리케이션이 종료되거나 중단될 때, 사용 중이던 소켓을 닫기 위해 `close(pContext->socketFd);`를 호출하여 리소스를 반환하도록 처리했습니다.

