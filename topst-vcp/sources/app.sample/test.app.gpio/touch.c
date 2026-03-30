#include "touch.h"
#include <gpio.h>
#include <stdint.h>
// touch 센서 Signal 핀 gpio 설정
static const uint32_t touch_sig = GPIO_GPB(26);
void Touch_Run(void) {
    GPIO_Config(touch_sig, (GPIO_FUNC(0) | GPIO_INPUT | 
GPIO_INPUTBUF_EN | GPIO_PULLDN)); // GPIO Input 설정
    while(1) {
        if (GPIO_Get(touch_sig) == 1) {
            // 터치 시 문구 출력
            mcu_printf("touch detected\n");
            SAL_TaskSleep(1000); // 1초 Sleep
        }
        else {
            // 터치 안할 시 문구 출력
            mcu_printf("touch not detected\n");
            SAL_TaskSleep(1000); // 1초 Sleep
        }
    }
}