#include <adc.h>
#include <sal_api.h>
#include <stdint.h>
#include "pot.h"
#define ADC_TYPE     0
#define ADC_MODULE   0 //사용할 ADC Module 번호
#define ADC_CHANNEL   ADC_CHANNEL_3 //측정할 채널
  

void Potentiometer_Run(void)
{
    uint32_t adc_val = 0;  //  ADC 변환 결과(12bit data)를 저장할 변수
    ADC_Init(ADC_TYPE, ADC_MODULE);  // ADC 초기화
    while (1)
    {
        adc_val = ADC_Read(ADC_CHANNEL, ADC_MODULE, 0);  
// ADC 값 읽기
        adc_val &= 0xFFF;  // 상위 상태 비트 제거 -> 하위 12bit(실제 ADC 값)만 추출
        mcu_printf("[ADC] CH%d = %4d\n", ADC_CHANNEL, 
adc_val);  // 결과를 UART 콘솔로 출력
        SAL_TaskSleep(100);
    }     
}
