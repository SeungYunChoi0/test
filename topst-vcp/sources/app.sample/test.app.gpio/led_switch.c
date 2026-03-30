#include "led_switch.h"
#include <gpio.h>
#include <sal_api.h>
#include <stdint.h>

static const uint32_t Switch_pin = GPIO_GPB(10);
static const uint32_t led_pins[4] = {
    GPIO_GPB(25),   
    GPIO_GPB(28),
    GPIO_GPB(11),
    GPIO_GPB(27), // 4개 디지털 핀 GPIO 설정
};

static void LED_AllOff(void)
{
    for (int i = 0; i < 4; ++i)
    {
        GPIO_Set(led_pins[i], 0U);
    }
}

void LED_Init(void)
{
    for (int i = 0; i < 4; ++i) {
        GPIO_Config(led_pins[i], (GPIO_FUNC(0) | GPIO_OUTPUT));
    }

    LED_AllOff(); // GPIO 출력 설정, 초기 OFF

    GPIO_Config(Switch_pin, (GPIO_FUNC(0) | GPIO_INPUT | 
GPIO_INPUTBUF_EN | GPIO_PULLDN)); // GPIO 입력, Pulldown 설정
}

void LED_Switch_Run(void)
{
    LED_Init(); // 초기화

    while (1) {
        /* pulldown 기준: 미입력 0, 눌림 1 */
        if (GPIO_Get(Switch_pin) == 0) {
            SAL_TaskSleep(10);
            continue;
        }

        /* debounce */
        SAL_TaskSleep(30);
        if (GPIO_Get(Switch_pin) == 0) {
            continue;
        }

        for (int i = 0; i < 4; i++) {
            GPIO_Set(led_pins[i], 1); // 순서대로 ON
            SAL_TaskSleep(500);
        }

        for (int i = 3; i >= 0; i--) {
            GPIO_Set(led_pins[i], 0U); // 순서대로 OFF
            SAL_TaskSleep(500);
        }

        /* 버튼을 떼기 전까지 재실행 방지 */
        while (GPIO_Get(Switch_pin) != 0) {
            SAL_TaskSleep(10);
        }
    }
}
