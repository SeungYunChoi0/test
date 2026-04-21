/*
 * custom_postproc.c
 * YOLOv8n-pose Hand Keypoint 후처리 (hand640_qt / rtpm3 연동)
 *
 * ┌──────────────────────────────────────────────────────────────────┐
 * │ 전체 흐름                                                         │
 * │  NPU 출력 (9개 텐서)                                              │
 * │    ↓                                                             │
 * │  [1] cls score 필터링  → sigmoid → CONF_THRESH 이하 제거          │
 * │  [2] DFL bbox 디코딩   → softmax → dist → xyxy 좌표              │
 * │  [3] keypoint 디코딩   → 21개 × (x, y, conf)                     │
 * │    ↓                                                             │
 * │  [4] NMS               → score 내림차순 정렬 → IoU 기반 중복 제거  │
 * │  [5] void *result(custom_hand_t*) 채우기 → NnRtpm이 JSON 전송    │
 * │  [6] stdout 로그 출력                                             │
 * └──────────────────────────────────────────────────────────────────┘
 *
 * NPU 출력 텐서 구조 (oact_entry_table.py 기준 최종 확정):
 *   tensor[0] Conv2d_45 (cv4_0): 1×63×80×80  P3 keypoint  stride=8
 *   tensor[1] Conv2d_43 (cv2_0): 1×64×80×80  P3 DFL bbox  stride=8
 *   tensor[2] Conv2d_44 (cv3_0): 1× 1×80×80  P3 cls/obj   stride=8
 *   tensor[3] Conv2d_59 (cv4_1): 1×63×40×40  P4 keypoint  stride=16
 *   tensor[4] Conv2d_57 (cv2_1): 1×64×40×40  P4 DFL bbox  stride=16
 *   tensor[5] Conv2d_58 (cv3_1): 1× 1×40×40  P4 cls/obj   stride=16
 *   tensor[6] Conv2d_71 (cv4_2): 1×63×20×20  P5 keypoint  stride=32
 *   tensor[7] Conv2d_69 (cv2_2): 1×64×20×20  P5 DFL bbox  stride=32
 *   tensor[8] Conv2d_70 (cv3_2): 1× 1×20×20  P5 cls/obj   stride=32
 *
 * 모델 입력 크기 : 640x640
 * 총 앵커 수     : 80x80 + 40x40 + 20x20 = 6400 + 1600 + 400 = 8400
 * keypoint 수   : 21개 (손목 1 + 손가락 4x5)
 * DFL reg_max   : 16
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "enlight_network.h"

/* ──────────────────────────────────────────────
 * 로그 매크로
 * ────────────────────────────────────────────── */
#ifdef BARE_METAL_FW_DEV
    extern int _printf(const char *format, ...);
#   define ENLIGHT_CUSTOM_PRINT  _printf
#else
#   define ENLIGHT_CUSTOM_PRINT  printf
#endif
#define enlight_custom_log(...) do { ENLIGHT_CUSTOM_PRINT(__VA_ARGS__); } while(0)

/* ──────────────────────────────────────────────
 * 하이퍼파라미터
 * ────────────────────────────────────────────── */
#define CONF_THRESH     0.3f
#define IOU_THRESH      0.45f
#define MAX_DET         2
#define DFL_REG_MAX     16
#define NUM_KPT         21
#define KPT_DIM         3
#define KPT_CH          (NUM_KPT * KPT_DIM)    /* 63 */
#define BBOX_CH         (4 * DFL_REG_MAX)       /* 64 */
#define NUM_SCALES      3
#define INPUT_W         640
#define INPUT_H         640
#define MAX_CANDIDATES  200

/* ──────────────────────────────────────────────
 * 스케일별 파라미터 (640x640 기준)
 *   P3: 640/8  = 80x80
 *   P4: 640/16 = 40x40
 *   P5: 640/32 = 20x20
 * ────────────────────────────────────────────── */
static const int SCALE_H[NUM_SCALES]      = {80, 40, 20};
static const int SCALE_W[NUM_SCALES]      = {80, 40, 20};
static const int SCALE_STRIDE[NUM_SCALES] = { 8, 16, 32};

/* ──────────────────────────────────────────────
 * 텐서 인덱스 매크로
 * oact_entry_table.py 기준 최종 확정 (640 모델):
 *   스케일 s=0(P3), 1(P4), 2(P5) 각각
 *   kpt = s*3+0, bbox = s*3+1, cls = s*3+2
 * ────────────────────────────────────────────── */
#define TENSOR_KPT(s)   ((s) * 3 + 0)   /* 0, 3, 6 */
#define TENSOR_BBOX(s)  ((s) * 3 + 1)   /* 1, 4, 7 */
#define TENSOR_CLS(s)   ((s) * 3 + 2)   /* 2, 5, 8 */

/* ──────────────────────────────────────────────
 * 내부 데이터 구조체 (후보 풀용)
 * NnType.h 의 hpd_keypoint_t / hpd_detection_t 와 동일 레이아웃
 * ────────────────────────────────────────────── */
typedef struct {
    float x;
    float y;
    float conf;
} keypoint_t;

typedef struct {
    float x1, y1, x2, y2;
    float score;
    keypoint_t kpts[NUM_KPT];
} hand_detection_t;

typedef struct {
    hand_detection_t dets[MAX_CANDIDATES];
    int n;
} candidate_pool_t;

/* post_process.c 에서 정의된 결과 구조체 (NnType.h 와 동일) */
typedef struct {
    float x;
    float y;
    float conf;
} hpd_keypoint_t;

typedef struct {
    float x1, y1, x2, y2;
    float score;
    hpd_keypoint_t kpts[NUM_KPT];
} hpd_detection_t;

typedef struct {
    hpd_detection_t dets[MAX_DET];
    int num_det;
} custom_hand_t;

/* ──────────────────────────────────────────────
 * 수학 유틸
 * ────────────────────────────────────────────── */
static inline float sigmoid(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

static void softmax_inplace(float *x, int n)
{
    int i;
    float max_val = -FLT_MAX;
    float sum = 0.0f;
    for (i = 0; i < n; i++)
        if (x[i] > max_val) max_val = x[i];
    for (i = 0; i < n; i++) {
        x[i] = expf(x[i] - max_val);
        sum += x[i];
    }
    for (i = 0; i < n; i++)
        x[i] /= sum;
}

static float calc_iou(const hand_detection_t *a, const hand_detection_t *b)
{
    float ix1 = (a->x1 > b->x1) ? a->x1 : b->x1;
    float iy1 = (a->y1 > b->y1) ? a->y1 : b->y1;
    float ix2 = (a->x2 < b->x2) ? a->x2 : b->x2;
    float iy2 = (a->y2 < b->y2) ? a->y2 : b->y2;
    float inter_w = ix2 - ix1;
    float inter_h = iy2 - iy1;
    if (inter_w <= 0.0f || inter_h <= 0.0f) return 0.0f;
    float inter_area = inter_w * inter_h;
    float area_a = (a->x2 - a->x1) * (a->y2 - a->y1);
    float area_b = (b->x2 - b->x1) * (b->y2 - b->y1);
    return inter_area / (area_a + area_b - inter_area);
}

/* ──────────────────────────────────────────────
 * DFL bbox 디코딩
 * ────────────────────────────────────────────── */
static void decode_dfl_bbox(
    const float *raw_bbox,
    int grid_x, int grid_y, int stride,
    float *out_xyxy)
{
    int d, i;
    float dist[4];
    float bins[DFL_REG_MAX];

    for (d = 0; d < 4; d++) {
        for (i = 0; i < DFL_REG_MAX; i++)
            bins[i] = raw_bbox[d * DFL_REG_MAX + i];
        softmax_inplace(bins, DFL_REG_MAX);
        dist[d] = 0.0f;
        for (i = 0; i < DFL_REG_MAX; i++)
            dist[d] += bins[i] * (float)i;
    }

    float cx = ((float)grid_x + 0.5f) * (float)stride;
    float cy = ((float)grid_y + 0.5f) * (float)stride;

    out_xyxy[0] = cx - dist[0] * (float)stride;
    out_xyxy[1] = cy - dist[1] * (float)stride;
    out_xyxy[2] = cx + dist[2] * (float)stride;
    out_xyxy[3] = cy + dist[3] * (float)stride;
}

/* ──────────────────────────────────────────────
 * keypoint 디코딩
 * 공식: kpt_x = (rx * 2.0 + grid_x) * stride
 *       kpt_y = (ry * 2.0 + grid_y) * stride
 * 결과: [0, INPUT_W] x [0, INPUT_H] 클리핑
 * ────────────────────────────────────────────── */
static void decode_keypoints(
    const float *raw_kpt,
    int grid_x, int grid_y, int stride,
    keypoint_t *out_kpts)
{
    int k;
    for (k = 0; k < NUM_KPT; k++) {
        float rx = raw_kpt[k * KPT_DIM + 0];
        float ry = raw_kpt[k * KPT_DIM + 1];
        float rc = raw_kpt[k * KPT_DIM + 2];

        float x = (rx * 2.0f + (float)grid_x) * (float)stride;
        float y = (ry * 2.0f + (float)grid_y) * (float)stride;

        if (x < 0.0f)           x = 0.0f;
        if (x > (float)INPUT_W) x = (float)INPUT_W;
        if (y < 0.0f)           y = 0.0f;
        if (y > (float)INPUT_H) y = (float)INPUT_H;

        out_kpts[k].x    = x;
        out_kpts[k].y    = y;
        out_kpts[k].conf = sigmoid(rc);
    }
}

/* ──────────────────────────────────────────────
 * NMS
 * score 내림차순 정렬 후 IoU 기반 억제
 * static 배열 사용 (malloc 불필요, 임베디드 안전)
 * ────────────────────────────────────────────── */
static int nms(candidate_pool_t *pool,
               hand_detection_t *result,
               int max_det)
{
    int i, j;
    int n = pool->n;
    static int suppressed[MAX_CANDIDATES];

    /* score 내림차순 버블 정렬 */
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            if (pool->dets[j].score > pool->dets[i].score) {
                hand_detection_t tmp = pool->dets[i];
                pool->dets[i] = pool->dets[j];
                pool->dets[j] = tmp;
            }
        }
    }

    memset(suppressed, 0, n * sizeof(int));
    int num_result = 0;

    for (i = 0; i < n && num_result < max_det; i++) {
        if (suppressed[i]) continue;
        result[num_result++] = pool->dets[i];
        for (j = i + 1; j < n; j++) {
            if (!suppressed[j] &&
                calc_iou(&pool->dets[i], &pool->dets[j]) > IOU_THRESH)
                suppressed[j] = 1;
        }
    }

    return num_result;
}

/* ──────────────────────────────────────────────
 * 초기화
 * ────────────────────────────────────────────── */
void custom_postproc_init(void)
{
}

/* ──────────────────────────────────────────────
 * 메인 후처리
 *
 * @param num_output     텐서 개수 (9개)
 * @param output_tensors ENLIGHT SDK 텐서 포인터 배열
 * @param result         custom_hand_t* cast – NnNeuralNetwork.c 에서
 *                       &pNeuralNetwork->hand_data 로 전달됨
 *
 * @return 검출된 손 개수 (0~MAX_DET)
 * ────────────────────────────────────────────── */
int custom_postproc_run(int num_output,
                        enlight_act_tensor_t **output_tensors,
                        void *result)
{
    int s, h, w, c, k, i;
    (void)num_output;  /* ENLIGHT API 요구 시그니처 */

    /* ─────────────────────────────────────────
     * result 포인터 캐스팅
     *   NnNeuralNetwork.c: network_run_postprocess(..., &hand_data)
     *   post_process.c   : custom_postproc_run(..., reserved)
     * ───────────────────────────────────────── */
    custom_hand_t *hand_result = (custom_hand_t *)result;

    /* static 선언 필수:
     * MAX_CANDIDATES x sizeof(hand_detection_t) 약 54KB
     * 로컬 선언 시 스택 오버플로우 발생 */
    static candidate_pool_t pool;
    pool.n = 0;

    /* ─────────────────────────────────────────
     * STEP 1~2: 스케일별 그리드 셀 순회 및 디코딩
     * ───────────────────────────────────────── */
    for (s = 0; s < NUM_SCALES; s++) {

        int H      = SCALE_H[s];
        int W      = SCALE_W[s];
        int stride = SCALE_STRIDE[s];

        enlight_act_tensor_t *t_bbox = output_tensors[TENSOR_BBOX(s)];
        enlight_act_tensor_t *t_cls  = output_tensors[TENSOR_CLS(s)];
        enlight_act_tensor_t *t_kpt  = output_tensors[TENSOR_KPT(s)];

        for (h = 0; h < H; h++) {
            for (w = 0; w < W; w++) {

                float raw_cls   = enlight_get_tensor_data_by_off(t_cls, h, w, 0);
                float obj_score = sigmoid(raw_cls);

                if (obj_score < CONF_THRESH) continue;
                if (pool.n >= MAX_CANDIDATES) continue;

                /* DFL bbox */
                float raw_bbox[BBOX_CH];
                for (c = 0; c < BBOX_CH; c++)
                    raw_bbox[c] = enlight_get_tensor_data_by_off(t_bbox, h, w, c);

                float xyxy[4];
                decode_dfl_bbox(raw_bbox, w, h, stride, xyxy);

                if (xyxy[0] < 0.0f)           xyxy[0] = 0.0f;
                if (xyxy[1] < 0.0f)           xyxy[1] = 0.0f;
                if (xyxy[2] > (float)INPUT_W) xyxy[2] = (float)INPUT_W;
                if (xyxy[3] > (float)INPUT_H) xyxy[3] = (float)INPUT_H;
                if (xyxy[2] <= xyxy[0] || xyxy[3] <= xyxy[1]) continue;

                /* keypoint */
                float raw_kpt[KPT_CH];
                for (c = 0; c < KPT_CH; c++)
                    raw_kpt[c] = enlight_get_tensor_data_by_off(t_kpt, h, w, c);

                hand_detection_t *det = &pool.dets[pool.n];
                det->x1    = xyxy[0];
                det->y1    = xyxy[1];
                det->x2    = xyxy[2];
                det->y2    = xyxy[3];
                det->score = obj_score;
                decode_keypoints(raw_kpt, w, h, stride, det->kpts);

                pool.n++;
            }
        }
    }

    /* ─────────────────────────────────────────
     * STEP 3: NMS
     * ───────────────────────────────────────── */
    hand_detection_t results[MAX_DET];
    int num_det = nms(&pool, results, MAX_DET);

    /* ─────────────────────────────────────────
     * STEP 4: 결과를 custom_hand_t 구조체에 기입
     *   → NnRtpm.c의 RtpmSendHandResultAsJson() 이
     *     hand_data.num_det / hand_data.dets[]를 읽어 JSON 전송
     * ───────────────────────────────────────── */
    if (hand_result != NULL) {
        hand_result->num_det = num_det;
        for (i = 0; i < num_det; i++) {
            hand_result->dets[i].x1    = results[i].x1;
            hand_result->dets[i].y1    = results[i].y1;
            hand_result->dets[i].x2    = results[i].x2;
            hand_result->dets[i].y2    = results[i].y2;
            hand_result->dets[i].score = results[i].score;
            for (k = 0; k < NUM_KPT; k++) {
                hand_result->dets[i].kpts[k].x    = results[i].kpts[k].x;
                hand_result->dets[i].kpts[k].y    = results[i].kpts[k].y;
                hand_result->dets[i].kpts[k].conf = results[i].kpts[k].conf;
            }
        }
    }

    /* ─────────────────────────────────────────
     * STEP 5: stdout 로그 (디버그용)
     * ───────────────────────────────────────── */
    for (i = 0; i < num_det; i++) {
        hand_detection_t *d = &results[i];
        enlight_custom_log(
            "[HAND %d] bbox: %.1f %.1f %.1f %.1f score: %.4f\n",
            i, d->x1, d->y1, d->x2, d->y2, d->score);
        for (k = 0; k < NUM_KPT; k++) {
            enlight_custom_log(
                "  KPT %d: (%.1f, %.1f) conf=%.4f\n",
                k, d->kpts[k].x, d->kpts[k].y, d->kpts[k].conf);
        }
    }

    if (num_det == 0)
        enlight_custom_log("[HAND] NO_DET\n");

    return num_det;
}