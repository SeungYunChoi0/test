
/*
    Openedges Enlight Network Compiler
    Hand Pose Detection (HPD) post_process.c
    - custom_postproc_run()에 void *result (custom_hand_t*) 전달
*/

#include "stdint.h"
#include <string.h>
#include "enlight_network.h"

#ifdef __i386__
#   include <stdio.h>
#   define post_process_log(...) do {printf(__VA_ARGS__);} while(0)
#else
#   define post_process_log(...) do {_printf(__VA_ARGS__);} while(0)
#endif

extern void custom_postproc_init();

extern int custom_postproc_run(
    int num_output,
    enlight_act_tensor_t**  outputs,          /**< output conf tensor number   */
    void *result                             /**< custom_hand_t* result buffer */
    );

#define HPD_MAX_DET 2
#define HPD_NUM_KPT 21

typedef struct {
    float x;
    float y;
    float conf;
} hpd_keypoint_t;

typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    hpd_keypoint_t kpts[HPD_NUM_KPT];
} hpd_detection_t;

typedef struct {
    hpd_detection_t dets[HPD_MAX_DET];
    int num_det;
} custom_hand_t;


/** @brief Run custom post-processing
 *
 *      Hand Pose Detection (YOLOv8n-pose / hand640_qt)
 *      reserved → custom_hand_t* : bbox + 21 keypoints per hand
 *
 *  @param net_inst         Network instance.
 *  @param output_base      NPU output tensor buffer base
 *  @param num_input        (unused)
 *  @param reserved         Pointer to custom_hand_t filled by custom_postproc_run()
 *
 *  @return Number of output tensors
 */
int run_post_process(
    void *net_inst,
    void *output_base,
    int  num_input,
    void *reserved)
{
    enlight_network_t *inst;
    enlight_custom_postproc_t* custom_param;

    int result;
    int i, num_output;
    enlight_act_tensor_t* output_tensors[MAX_NUM_OUTPUT];

    inst = (enlight_network_t *)net_inst;
    custom_param = (enlight_custom_postproc_t*)inst->post_proc_extension;

    /* reserved(custom_hand_t*)를 미리 초기화 */
    if (reserved != NULL)
        memset(reserved, 0, sizeof(custom_hand_t));

    num_output = enlight_custom_get_output_tensors(custom_param, output_tensors);

    for (i = 0; i < num_output; i++)
        output_tensors[i]->base = output_base;

    custom_postproc_init();

    /* void *result 로 custom_hand_t* 전달 */
    result = custom_postproc_run(num_output, output_tensors, reserved);
    post_process_log("custom process result: %d\n", result);

    return num_output;
}
