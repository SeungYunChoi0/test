#if ( MCU_BSP_SUPPORT_CAN_DEMO == 1 )

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <app_cfg.h>

#include "bsp.h"
#include "gic.h"
#include "gpio.h"
#include "debug.h"
#include "pdm.h"
#include "i2c.h"
#include "stdio.h"

#include "can_vcp_ctrl.h"
#define MIN_DUTY      (35) //PDM 최소 듀티 비(%)

/**************************************************************************************************
*                                           LOCAL FUNCTIONS
**************************************************************************************************/
static PDMModeConfig_t cfgA, cfgB; //2개의 모터 설정 구조체 저장용

static void BrakeLED_ON(void); //브레이크등 제어 함수 프로토 타입
static void BrakeLED_OFF(void);
static void LeftSignalLED_ON(void); //방향등(좌우) LED제어 함수 프로토타입
static void LeftSignalLED_OFF(void);
static void RightSignalLED_ON(void);
static void RightSignalLED_OFF(void);
static void HeadLED_ON(void); //전조등 LED제어 함수 프로토타입
static void HeadLED_OFF(void);

static void lcd_send(uint8 mode, uint8 data); //LCD 제어용 I2C 전송 관련 함수
static void lcd_cmd(uint8 cmd);
static void lcd_data(uint8 dat);
static void lcd_init(void);
static void lcd_print(const char *str);

static void pin_out(uint32 p); //GPIO 출력 유틸리티 함수
static void pin_hi (uint32 p);
static void pin_lo (uint32 p);

static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg); //모터용 PDM 제어 내부 함수
static void MotorPDM_Init(void);
static void MotorA_Set(uint32 duty_pct, uint32 forward);
static void MotorB_Set(uint32 duty_pct, uint32 forward);
static uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns);
static sint8 duty_from_speed(sint8 speed);

static void ControlBrakeLight(boolean bTurnOn); //센서 제어 함수
static void ControlSignalLight(boolean bLeft, boolean bTurnOn);
static void ControlHeadLight(boolean bTurnOn);
static void ControlFuelLevel(uint8 fuelLevel);

static void ConfigureServoPDM(uint32 channel, uint32 port, uint32 angle_deg);

/**************************************************************************************************
*                                           LOCAL IMPLEMENTATIONS
**************************************************************************************************/

/* [PDF p.14] InitSensorControls: 모터 PDM + I2C + LCD 초기화 */
boolean InitSensorControls(void) 
{
    MotorPDM_Init();

    I2C_Init();
    if (I2C_Open(I2C_CH, I2C_PORT, I2C_SPEED, NULL_PTR, NULL_PTR) != SAL_RET_SUCCESS)
    {
        mcu_printf("[LCD] I2C_Open failed (ch=%d, port=%d, speed=%d)\r\n",
                   (int)I2C_CH, (int)I2C_PORT, (int)I2C_SPEED);
        return FALSE;
    }
    I2C_ScanSlave(I2C_CH);
    lcd_init();
    return TRUE;
}


/* ------------------------------- Brake Light ------------------------------- */

/* [PDF p.14] GPIO_Set으로 실제 핀 제어 */
static void BrakeLED_ON(void)
{
    GPIO_Set(BrakeLEDPIN, 1); //브레이크등 ON
}

static void BrakeLED_OFF(void) //브레이크등 OFF
{
    GPIO_Set(BrakeLEDPIN, 0);
}

/* [PDF p.15] 최초 호출 시 GPIO_Config 설정 후 ON/OFF */
static void ControlBrakeLight(boolean bTurnOn)
{
    static boolean binit = FALSE; //핀 초기화 여부

    mcu_printf("[BRAKE] Controlling Brake Light\r\n");
    if (binit == FALSE) //아직 초기화 안된 경우
    {
        GPIO_Config(BrakeLEDPIN,
                    GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        BrakeLED_OFF(); //초기 상태 OFF
        binit = TRUE;
    }
    // 브레이크 등 On OFF
    if (bTurnOn)
    {
        BrakeLED_ON();
        mcu_printf("[BRAKE] ON\r\n");
    }
    else
    {
        BrakeLED_OFF();
        mcu_printf("[BRAKE] OFF\r\n");
    }
}


/* -------------------------- Turn Signal Light -------------------------- */

/* [PDF p.15] GPIO_Set으로 실제 핀 제어 */
static void LeftSignalLED_ON(void)
{
    GPIO_Set(LeftSignalLEDPIN, 1);//좌측 방향등 ON
}

static void LeftSignalLED_OFF(void) //좌측 방향등 OFF
{
    GPIO_Set(LeftSignalLEDPIN, 0);
}

/* [PDF p.16] GPIO_Set으로 실제 핀 제어 */
static void RightSignalLED_ON(void)
{
    GPIO_Set(RightSignalLEDPIN, 1); //우측 방향등  ON
}

static void RightSignalLED_OFF(void) 
{
    GPIO_Set(RightSignalLEDPIN, 0);//우측 방향등 OFF
}

/* [PDF p.16-17] 최초 호출 시 좌·우 GPIO_Config 후 방향별 제어 */
static void ControlSignalLight(boolean bLeft, boolean bTurnOn)
{
    static boolean binit = FALSE;

    mcu_printf("[SIGNAL] Controlling Signal Light\r\n");
    if (binit == FALSE)
    { //좌우 방향등 GPIO 설정
        GPIO_Config(LeftSignalLEDPIN,
                    GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        GPIO_Config(RightSignalLEDPIN,
                    GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        LeftSignalLED_OFF();
        RightSignalLED_OFF();
        binit = TRUE;
    }
    //방향별로 제어
    if (bLeft)
    {
        if (bTurnOn)
        {
            LeftSignalLED_ON();
            mcu_printf("[LeftSignal] ON\r\n");
        }
        else
        {
            LeftSignalLED_OFF();
            mcu_printf("[LeftSignal] OFF\r\n");
        }
    }
    else
    {
        if (bTurnOn)
        {
            RightSignalLED_ON();
            mcu_printf("[RightSignal] ON\r\n");
        }
        else
        {
            RightSignalLED_OFF();
            mcu_printf("[RightSignal] OFF\r\n");
        }
    }
}


/* ------------------------------- Head Light ------------------------------- */

/* [PDF p.18] GPIO_Set으로 실제 핀 제어 */
static void HeadLED_ON(void)
{
    GPIO_Set(HeadLEDPIN, 1); //전조등 ON
}

static void HeadLED_OFF(void) 
{
    GPIO_Set(HeadLEDPIN, 0); //전조등 OFF
}

/* [PDF p.18] 최초 호출 시 GPIO_Config 설정 후 ON/OFF */
static void ControlHeadLight(boolean bTurnOn)
{
    static boolean binit = FALSE;

    mcu_printf("[LIGHT] Controlling Head Light\r\n");
    if (binit == FALSE)
    {
        GPIO_Config(HeadLEDPIN,
                    GPIO_FUNC(0) | GPIO_OUTPUT | GPIO_NOPULL | GPIO_DS(3) | GPIO_INPUTBUF_DIS);
        HeadLED_OFF();
        binit = TRUE;
    }

    if (bTurnOn)
    {
        HeadLED_ON();
        mcu_printf("[Head Light] ON\r\n");
    }
    else
    {
        HeadLED_OFF();
        mcu_printf("[Head Light] OFF\r\n");
    }
}


/* ------------------------------- Fuel Level (I2C LCD) ------------------------------- */

/*
 * [PDF p.19] lcd_send: HD44780 4-bit 모드 I2C 전송
 * PCF8574 I2C expander를 통해 6바이트 EN 스트로브 시퀀스 전송
 * buf[]: EN=Low→High→Low 순서로 EN 비트 토글
 */
static void lcd_send(uint8 mode, uint8 data)
{
    uint8 high = data & 0xF0;
    uint8 low  = (data << 4) & 0xF0;
    uint8 buf[6];
    //LCD 제어 비트
    buf[0] = high | 0x08 | (mode ? 0x01 : 0x00);  // EN=0
    buf[1] = high | 0x0C | (mode ? 0x01 : 0x00);  // EN=1
    buf[2] = high | 0x08 | (mode ? 0x01 : 0x00);  // EN=0
    buf[3] = low  | 0x08 | (mode ? 0x01 : 0x00);  // EN=0
    buf[4] = low  | 0x0C | (mode ? 0x01 : 0x00);  // EN=1
    buf[5] = low  | 0x08 | (mode ? 0x01 : 0x00);  // EN=0

    I2CXfer_t xfer = {
        .xCmdLen = 0,
        .xOutLen = 6,
        .xOutBuf = buf,
        .xInLen  = 0,
        .xInBuf  = NULL,
        .xCmdBuf = NULL,
        .xOpt    = 0
    };

    (void)I2C_Xfer(I2C_CH, (uint8)(LCD_ADDR << 1), xfer, 0);
    SAL_TaskSleep(2);
}

/* [PDF p.20] */
static void lcd_cmd(uint8 cmd)
{
    lcd_send(LCD_CMD, cmd); //LCD 제어 명령어 전송
}

static void lcd_data(uint8 dat)
{
    lcd_send(LCD_DATA, dat); //데이터 전송
}

/* [PDF p.20] HD44780 4-bit 모드 초기화 시퀀스 */
static void lcd_init(void)
{
    SAL_TaskSleep(50);   // 전원 안정화
    lcd_cmd(0x33);       // 초기화
    lcd_cmd(0x32);       // 4-bit 모드 전환
    lcd_cmd(0x28);       // 4-bit, 2-line, 5x8 폰트
    lcd_cmd(0x0C);       // 디스플레이 ON, 커서 OFF
    lcd_cmd(0x06);       // 커서 오른쪽 이동
    lcd_cmd(0x01);       // 화면 클리어
    SAL_TaskSleep(5);
}

/* [PDF p.20] */
static void lcd_print(const char *str)
{
    while (*str) lcd_data((uint8)*str++); //문자열을 한 글자씩 LCD에 전송
}

/* [PDF p.21] 값 변화 없으면 LCD 재출력 생략 (I2C 트랜잭션 절약) */
static void ControlFuelLevel(uint8 fuelLevel)
{
    static uint8 prev = 0xFF; //이전 값 저장(불필요한 재출력 방지)
    uint8 pct = (fuelLevel > 100) ? 100 : fuelLevel; //0~100% 범위 제한

    if (pct == prev) //변화 없으면 종료
    {
        return;
    }
    prev = pct;

    lcd_cmd((uint8)(0x80 | 0x00));   // 1번째 줄
    lcd_print("Fuel Level      ");

    char line[17];
    (void)snprintf(line, sizeof(line), "Fuel: %3u%%      ", (unsigned)pct);
    lcd_cmd((uint8)(0x80 | 0x40));   // 2번째 줄
    lcd_print(line);

    mcu_printf("[FUEL] Controlling Fuel Level : %d\r\n", pct);
}


/* -------------------------- Motor with PWM -------------------------- */

/* [PDF p.22] GPIO 출력 유틸리티 */
static void pin_out(uint32 p) { GPIO_Config(p, GPIO_OUTPUT | GPIO_FUNC(0) | GPIO_DS(3)); }
static void pin_hi(uint32 p)  { GPIO_Set(p, 1); }
static void pin_lo(uint32 p)  { GPIO_Set(p, 0); }

/*
 * [PDF p.22] pdm_apply: 채널 비활성화 → 설정 적용 → 재활성화
 * 100ms 타임아웃으로 채널 종료 대기
 */
static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg)
{
    uint32 wait = 0;

    (void)PDM_Disable(ch, PMM_OFF);
    while (PDM_GetChannelStatus(ch) && wait < 100)
    {
        SAL_TaskSleep(1);
        wait++;
    }

    if (PDM_SetConfig(ch, cfg) != SAL_RET_SUCCESS) return -1; //설정 적용
    if (PDM_Enable(ch, PMM_OFF)  != SAL_RET_SUCCESS) return -2; //채널 활성화
    return 0;
}

/*
 * [PDF p.22] duty_pct_to_ns: 듀티(%) → 나노초 변환
 * MIN_DUTY(35%) 미만이면 MIN_DUTY로 클램프 (모터 최소 구동 보장)
 * uint64 사용으로 오버플로 방지
 */
static uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns)
{
    if (pct == 0) return 0;
    if (pct > 100) pct = 100;
    if (pct < MIN_DUTY && pct > 0) pct = MIN_DUTY;

    uint64 num = (uint64)period_ns * (uint64)pct + 50ULL;
    return (uint32)(num / 100ULL);
}

/*
 * [PDF p.23] duty_from_speed: 속도(1~80) → 듀티(%) 선형 매핑
 * 공식: duty = 40 + ((speed-1) * 80) / 79
 *   speed=1  → 40%
 *   speed=80 → ~121% (내부에서 100%로 클램프됨)
 */
static sint8 duty_from_speed(sint8 speed)
{
    if (speed == 0) { return 0; }
    if (speed < 0)  { return 0; }
    if (speed > 80) { speed = 80; }

    return (sint8)(40 + ((speed - 1) * 80) / 79);
}

/* [PDF p.23-24] MotorPDM_Init: IN1~IN4 GPIO + PDM A·B 초기화 (1회만) */
static void MotorPDM_Init(void) //PDM 초기화 함수
{
    static boolean inited = FALSE;
    if (inited) return;

    /* GPIO 초기화 */
    pin_out(IN1); pin_out(IN2);
    pin_out(IN3); pin_out(IN4);
    pin_lo(IN1);  pin_lo(IN2);
    pin_lo(IN3);  pin_lo(IN4);

    /* PDM 초기화 */
    PDM_Init();
    PDM_CfgSetWrPw();
    PDM_CfgSetWrLock(0);

    /* 모터 A 설정 (ENA_SEL=0, ENA_PORT=PERICH_CH0) */
    SAL_MemSet(&cfgA, 0, sizeof(cfgA));
    cfgA.mcPortNumber     = ENA_PORT;
    cfgA.mcOperationMode  = PDM_OUTPUT_MODE_PHASE_1;
    cfgA.mcPeriodNanoSec1 = PDM_PERIOD_NS;
    cfgA.mcDutyNanoSec1   = 0;
    (void)pdm_apply(ENA_SEL, &cfgA);

    /* 모터 B 설정 (ENB_SEL=4, ENB_PORT=PERICH_CH1) */
    SAL_MemSet(&cfgB, 0, sizeof(cfgB));
    cfgB.mcPortNumber     = ENB_PORT;
    cfgB.mcOperationMode  = PDM_OUTPUT_MODE_PHASE_1;
    cfgB.mcPeriodNanoSec1 = PDM_PERIOD_NS;
    cfgB.mcDutyNanoSec1   = 0;
    (void)pdm_apply(ENB_SEL, &cfgB);

    inited = TRUE;
}

/*
 * [PDF p.25] MotorA_Set
 * IN1=0, IN2=1 → 정방향
 * IN1=1, IN2=0 → 역방향
 * duty_pct=0   → 정지 (IN1=IN2=0)
 */
static void MotorA_Set(uint32 duty_pct, uint32 forward) //모터 A 제어
{
    if (duty_pct == 0)
    {
        pin_lo(IN1); pin_lo(IN2); //정지
    }
    else if (forward)
    {
        pin_lo(IN1); pin_hi(IN2); //정방향
    }
    else
    {
        pin_hi(IN1); pin_lo(IN2); //역방향
    }
    cfgA.mcDutyNanoSec1 = duty_pct_to_ns(duty_pct, cfgA.mcPeriodNanoSec1);
    (void)pdm_apply(ENA_SEL, &cfgA);
}

/*
 * [PDF p.25] MotorB_Set
 * IN3=1, IN4=0 → 정방향
 * IN3=0, IN4=1 → 역방향
 * duty_pct=0   → 정지
 */
static void MotorB_Set(uint32 duty_pct, uint32 forward) //모터 B제어
{
    if (duty_pct == 0)
    {
        pin_lo(IN3); pin_lo(IN4); //정지
    }
    else if (forward)
    {
        pin_hi(IN3); pin_lo(IN4); //정방향
    }
    else
    {
        pin_lo(IN3); pin_hi(IN4); //역방향
    }
    cfgB.mcDutyNanoSec1 = duty_pct_to_ns(duty_pct, cfgB.mcPeriodNanoSec1);
    (void)pdm_apply(ENB_SEL, &cfgB);
}

/*
 * [PDF p.26-27] ConfigureServoPDM: 서보모터 각도 제어
 * 주기 20ms(50Hz), 듀티: 0°→0.5ms, 90°→1.5ms, 180°→2.5ms
 * PDM ch5, GPIO_PERICH_CH3 사용
 */
static void ConfigureServoPDM(uint32 channel, uint32 port, uint32 angle_deg) //서브모터 설정 함수
{
    PDMModeConfig_t pwm_cfg;
    uint32 duty_ns  = 500000 + (angle_deg * (2000000 / 180));  // 0.5~2.5ms
    uint32 wait_cnt = 0;

    pwm_cfg.mcPortNumber      = port;
    pwm_cfg.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    pwm_cfg.mcInversedSignal  = 0;
    pwm_cfg.mcOutSignalInIdle = 0;
    pwm_cfg.mcLoopCount       = 0;
    pwm_cfg.mcOutputCtrl      = 0;
    pwm_cfg.mcPeriodNanoSec1  = 20000000;  // 20ms (50Hz)
    pwm_cfg.mcDutyNanoSec1    = duty_ns;
    pwm_cfg.mcPeriodNanoSec2  = 0;
    pwm_cfg.mcDutyNanoSec2    = 0;

    PDM_Disable(channel, PMM_ON); //PDM 재적용
    while (PDM_GetChannelStatus(channel))
    {
        SAL_TaskSleep(1);
        if (++wait_cnt > 100)
        {
            mcu_printf("Timeout on channel %d\n", channel);
            return;
        }
    }

    if (PDM_SetConfig(channel, &pwm_cfg) != SAL_RET_SUCCESS)
    {
        mcu_printf("SetConfig fail (CH:%d)\n", channel);
        return;
    }
    if (PDM_Enable(channel, PMM_ON) != SAL_RET_SUCCESS)
    {
        mcu_printf("Enable fail (CH:%d)\n", channel);
        return;
    }
    mcu_printf("CH%d angle: %3d deg -> duty: %d ns\n", channel, angle_deg, duty_ns);
}


/**************************************************************************************************
*                                     ControlBreadBoardSensors
* CAN 수신 메시지 ID → 해당 제어 함수 디스패치
**************************************************************************************************/
void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData) //CAN 메시지 처리 함수
{
    sint8 ucMsgData[2] = {0x00, 0x00};
    if (nDataLength == 0)
    {
        return; // 데이터 없으면 무시
    }

    switch (mId)
    {
        case VCP_IO_BREAK_LIGHT: // 브레이크등 0x101
        {
            ucMsgData[0] = pucData[0];
            if (ucMsgData[0] == VCP_IO_ACTION_ON)
            {
                ControlBrakeLight(TRUE);
            }
            else
            {
                ControlBrakeLight(FALSE);
            }
            break;
        }

        case VCP_IO_TURN_SIGNAL: // 방향등 0x102
        {
            ucMsgData[0] = pucData[0];
            ucMsgData[1] = pucData[1];
            if (ucMsgData[0] == VCP_IO_SUB_LEFT)
            {
                if (ucMsgData[1] == VCP_IO_ACTION_ON)
                {
                    ControlSignalLight(TRUE, TRUE);
                }
                else
                {
                    ControlSignalLight(TRUE, FALSE);
                }
            }
            if (ucMsgData[0] == VCP_IO_SUB_RIGHT)
            {
                if (ucMsgData[1] == VCP_IO_ACTION_ON)
                {
                    ControlSignalLight(FALSE, TRUE);
                }
                else
                {
                    ControlSignalLight(FALSE, FALSE);
                }
            }
            break;
        }

        case VCP_IO_EMER_SIGNAL: // 비상등 0x103 [PDF p.29: OFF 처리 추가]
        {
            ucMsgData[0] = pucData[0];
            if (ucMsgData[0] == VCP_IO_ACTION_ON)
            {
                ControlSignalLight(TRUE,  TRUE);
                ControlSignalLight(FALSE, TRUE);
            }
            else
            {
                ControlSignalLight(TRUE,  FALSE);
                ControlSignalLight(FALSE, FALSE);
            }
            break;
        }

        case VCP_IO_HEAD_LIGHT: // 전조등 0x104
        {
            ucMsgData[0] = pucData[0];
            if (ucMsgData[0] == VCP_IO_ACTION_ON)
            {
                ControlHeadLight(TRUE);
            }
            else
            {
                ControlHeadLight(FALSE);
            }
            break;
        }

        case VCP_IO_FUEL_LEVEL: // 연료게이지 0x105
        {
            ucMsgData[0] = (pucData[0] > 100) ? 100 : pucData[0];
            ControlFuelLevel((uint8)ucMsgData[0]);
            break;
        }

        case VCP_IO_MOTOR_SPEED: // DC 모터 속도 0x106 [PDF p.31: 실제 모터 제어 추가]
        {
            static sint8 duty = 0;
            sint8 speed = (sint8)pucData[0]; // 0~80

            if (speed == 0)
            {
                MotorA_Set(0, 1);
                MotorB_Set(0, 1);
            }
            else if (speed > 0)
            {
                duty = duty_from_speed(speed);
                MotorA_Set((uint32)duty, 1);
                MotorB_Set((uint32)duty, 1);
            }
            else
            {
                duty = duty_from_speed((sint8)(-speed));
                MotorA_Set((uint32)duty, 0);
                MotorB_Set((uint32)duty, 0);
            }
            mcu_printf("[MOTOR] speed=%d => duty=%d%%\r\n", (sint8)speed, (sint8)duty);
            break;
        }

        case VCP_IO_MOTOR_WHEEL: // 서보모터 조향 0x107 [PDF p.32: ConfigureServoPDM 호출 추가]
        {
            int8_t input = (int8_t)pucData[0]; // 0~127
            if (input < 0)   input = 0;
            if (input > 127) input = 127;
            // 입력 65 → 중앙(90°), 0 → 45°, 127 → 135°
            int angle = 90 + ((input - 65) * 45) / 65;
            ConfigureServoPDM(5, GPIO_PERICH_CH3, (uint32)angle);
            mcu_printf("[SERVO] input=%d -> angle=%d deg\r\n", input, angle);
            break;
        }

        default:
        {
            mcu_printf("[%s][%d] undefined can id !!!\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }
}

#endif
