#include <stdint.h>
#include <gpio.h>
#include <pdm.h>
#include "pdm_motor.h"

/* 서보 모터 설정을 위한 정적 함수 */
static void ConfigureServoPWM(uint32 channel, uint32 port, uint32 angle_deg) {
    PDMModeConfig_t pwm_cfg;
    /* 0~180도 -> 0.5~2.5ms (500,000~2,500,000ns) 계산 */
    uint32 duty_ns = 500000 + (angle_deg * (2000000 / 180)); 
    uint32 wait_cnt = 0;

    pwm_cfg.mcPortNumber = port;             // 출력 포트 설정
    pwm_cfg.mcOperationMode = PDM_OUTPUT_MODE_PHASE_1; // 단일 위상 출력 모드
    pwm_cfg.mcInversedSignal = 0;           // 출력 반전 비활성화
    pwm_cfg.mcOutSignalInIdle = 0;          // 유휴 시 신호 출력 없음
    pwm_cfg.mcLoopCount = 0;                // 루프 카운트(0: 지속 출력)
    pwm_cfg.mcOutputCtrl = 0;               // 출력 제어 설정 기본값

    pwm_cfg.mcPeriodNanoSec1 = 20000000;    // 주기 20ms (50Hz)
    pwm_cfg.mcDutyNanoSec1 = duty_ns;       // 듀티: 0.5~2.5ms
    pwm_cfg.mcPeriodNanoSec2 = 0;           // 사용 x
    pwm_cfg.mcDutyNanoSec2 = 0;             // 사용 x

    PDM_Disable(channel, PMM_ON);           // PDM 출력 비활성화

    /* PDM이 꺼질 때까지 대기 */
    while (PDM_GetChannelStatus(channel)) {
        SAL_TaskSleep(1);
        if (++wait_cnt > 100) {
            mcu_printf("Timeout on channel %d\n", channel);
            return;
        }
    }

    /* PDM 설정 적용 */
    if (PDM_SetConfig(channel, &pwm_cfg) != SAL_RET_SUCCESS) {
        mcu_printf("SetConfig fail (CH:%d)\n", channel);
        return;
    }

    /* PDM 활성화 */
    if (PDM_Enable(channel, PMM_ON) != SAL_RET_SUCCESS) {
        mcu_printf("Enable fail (CH:%d)\n", channel);
        return;
    }
}

/* 메인 태스크에서 호출할 진입 함수 */
void PDM_Motor_Run(void) {
    PDM_Init(); // PDM 초기화

    while (1) {
        /* 0 -> 180도 방향으로 회전 */
        for (int angle = 0; angle <= 180; angle += 5) {
            ConfigureServoPWM(3, GPIO_PERICH_CH0, angle); // 해당 각도로 PWM 신호 갱신
            // 필요한 경우 적절한 SAL_TaskSleep을 추가하여 동작 속도를 조절할 수 있습니다.
        }

        /* 180 -> 0도 방향으로 반대 회전 */
        for (int angle = 180; angle >= 0; angle -= 5) {
            ConfigureServoPWM(3, GPIO_PERICH_CH0, angle); // 각도 감소에 따라 듀티 변경
        }
    }
}