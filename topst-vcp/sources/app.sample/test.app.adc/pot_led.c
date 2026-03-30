#include <stdint.h>
#include <gpio.h>
#include <adc.h>
#include "pot_led.h"
#define ADC_TYPE      0
#define ADC_MODULE    0
#define ADC_CHANNEL   ADC_CHANNEL_3  //가변저항 입력

static const uint32_t led_pin=GPIO_GPB(25);
static void LED_Init(void)
{
        GPIO_Config(led_pin, (GPIO_FUNC(0) | GPIO_OUTPUT));  // 사용할 GPIO_GPB(25)핀 설정
        GPIO_Set(led_pin, 0);  // 초기 값 설정
}
void Potentiometer_LED_Run(void)
{
    uint32_t adc_val;
    LED_Init();
    ADC_Init(ADC_TYPE, ADC_MODULE);
    while(1)
    {
        adc_val = ADC_Read(ADC_CHANNEL, ADC_MODULE, 0);  // ADC 값 읽기
        adc_val &= 0xFFF; // 하위 12bit 유효값
            if (adc_val > 0 && adc_val < 1000) {  // 값이 0~1000이면 led off
                        GPIO_Set(led_pin, 0);
            }
            else if (adc_val >= 1000 && adc_val < 2000) {  // 값이 1000~2000이면 led on
                        GPIO_Set(led_pin, 1);
            }
            else if (adc_val >= 2000 && adc_val < 3000) {  // 값이 2000~3000이면 led off
                        GPIO_Set(led_pin, 0);
            }
            else if (adc_val >= 3000) {  // 값이 3000이상이면 led on
                        GPIO_Set(led_pin, 1);
            }
            else {
                        GPIO_Set(led_pin, 0);
            }
        mcu_printf("[ADC] ADC=%4d\n", adc_val);
        SAL_TaskSleep(100);
    }
}