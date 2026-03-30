
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
#define MIN_DUTY      (35) 

/**************************************************************************************************
*                                           LOCAL FUNCTIONS
**************************************************************************************************/
static PDMModeConfig_t cfgA, cfgB;

static void BrakeLED_ON(void);
static void BrakeLED_OFF(void);
static void LeftSignalLED_ON(void);
static void LeftSignalLED_OFF(void);
static void RightSignalLED_ON(void);
static void RightSignalLED_OFF(void);
static void HeadLED_ON(void);
static void HeadLED_OFF(void);

static void lcd_send(uint8 mode, uint8 data);
static void lcd_cmd(uint8 cmd);
static void lcd_data(uint8 dat);
static void lcd_init(void);
static void lcd_print(const char *str);

static void pin_out(uint32 p);
static void pin_hi (uint32 p);
static void pin_lo (uint32 p);

static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg);
static void MotorPDM_Init(void);
static void MotorA_Set(uint32 duty_pct, uint32 forward);
static void MotorB_Set(uint32 duty_pct, uint32 forward);
static uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns);
static sint8 duty_from_speed(sint8 speed);

static void ControlBrakeLight(boolean bTurnOn);
static void ControlSignalLight(boolean bLeft, boolean bTurnOn);
static void ControlHeadLight(boolean bTurnOn);
static void ControlFuelLevel(uint8 fuelLevel);

void InitSensorControlls(void);

/**************************************************************************************************
*                                           LOCAL IMPLEMENTATIONS
**************************************************************************************************/
boolean InitSensorControls(void)
{
    return TRUE;
}


/* ------------------------------- Brake Light ------------------------------- */
void BrakeLED_ON(void)  
{
    mcu_printf("Brake LED ON\r\n");
}

void BrakeLED_OFF(void)
{
    mcu_printf("Brake LED OFF\r\n");
}

void ControlBrakeLight(boolean bTurnOn)
{
    if(bTurnOn)
    {
        BrakeLED_ON();
    }
    else
    {
        BrakeLED_OFF();
    }
}

/* -------------------------- Turn Signal Light -------------------------- */
void LeftSignalLED_ON(void)   
{ 
    mcu_printf("Left Signal LED ON\r\n");
}

void LeftSignalLED_OFF(void)  
{
    mcu_printf("Left Signal LED oFF\r\n");
}

void RightSignalLED_ON(void)  
{
    mcu_printf("Right Signal LED ON\r\n"); 
}

void RightSignalLED_OFF(void) 
{
    mcu_printf("Right Signal LED OFF\r\n"); 
}

void ControlSignalLight(boolean bLeft, boolean bTurnOn)
{
    if(bLeft)
    {
        if(bTurnOn)
        {
            LeftSignalLED_ON();
        }
        else
        {
            LeftSignalLED_OFF();
        }
    }
    else
    {
        if(bTurnOn)
        {
            RightSignalLED_ON();
        }
        else
        {
            RightSignalLED_OFF();
        }
    }
}

/* ------------------------------- Head Light ------------------------------- */

void HeadLED_ON(void)  
{
    mcu_printf("Head LED ON\r\n");
}

void HeadLED_OFF(void)
{
    mcu_printf("Head LED OFF\r\n");
}

void ControlHeadLight(boolean bTurnOn)
{
    if(bTurnOn) {
        HeadLED_ON();
        mcu_printf("[Head Light] ON\r\n");
    }   else {
        HeadLED_OFF();
        mcu_printf("[Head Light] OFF\r\n");
    }
}

/* ------------------------------- Fuel Level ------------------------------- */

void lcd_send(uint8 mode, uint8 data) 
{
}

void lcd_cmd(uint8 cmd)  
{ 
}

void lcd_data(uint8 dat) 
{ 
}

void lcd_init(void) 
{
}

void lcd_print(const char *str) 
{
}

void ControlFuelLevel(uint8 fuelLevel)
{
    uint8 pct = (fuelLevel > 100) ? 100 : fuelLevel;
    mcu_printf("[FUEL] Controlling Fuel Level : %d\r\n", pct);
}

/* -------------------------- Motor with PWM -------------------------- */

static inline void pin_out(uint32 p) {}
static inline void pin_hi(uint32 p)  {}
static inline void pin_lo(uint32 p)  {}
static int pdm_apply(uint32 ch, PDMModeConfig_t* cfg)
{
    ( void ) ch;
    ( void ) cfg;

    return 0;
}

uint32 duty_pct_to_ns(uint32 pct, uint32 period_ns)
{
    return ( pct * period_ns ) / 100U;
}

sint8 duty_from_speed(sint8 speed)
{
    return speed;
}

void MotorPDM_Init(void)
{
}

void MotorA_Set(uint32 duty_pct, uint32 forward)
{
}

void MotorB_Set(uint32 duty_pct, uint32 forward)
{
}

static void ConfigureServoPDM(uint32 channel, uint32 port, uint32 angle_deg)
{
    ( void ) channel;
    ( void ) port;
    ( void ) angle_deg;
}

void ControlBreadBoardSensors(uint32 mId, uint8 nDataLength, sint8* pucData)
{
    sint8 ucMsgData[2] = {0x00, 0x00};
    if(nDataLength == 0)
    {
        return; // 데이터 없으면 무시
    }
    switch(mId)
    {
        case VCP_IO_BREAK_LIGHT: // 브레이크등 0x101
        {
            ucMsgData[0] = pucData[0];
            if(ucMsgData[0] == VCP_IO_ACTION_ON)
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
                if(ucMsgData[1] == VCP_IO_ACTION_ON)
                {
                    // turn on the left signal
                    ControlSignalLight(TRUE, TRUE);
                }
                else
                {
                    // turn off the left light
                    ControlSignalLight(TRUE, FALSE);
                }
            }            
            if (ucMsgData[0] == VCP_IO_SUB_RIGHT)
            { 
                if(ucMsgData[1] == VCP_IO_ACTION_ON)
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

        case VCP_IO_EMER_SIGNAL: //비상등 0x103
        {
            ucMsgData[0] = pucData [0];
            if(ucMsgData[0] == VCP_IO_ACTION_ON)
            {
                mcu_printf("Emergency Signal ON<->OFF\r\n");
            }
            break;
        }
        case VCP_IO_HEAD_LIGHT: //헤드라이트 0x104
        {
            ucMsgData[0] = pucData[0];
            if(ucMsgData[0] == VCP_IO_ACTION_ON)
            {
                ControlHeadLight(TRUE);
            }
            else
            {
                ControlHeadLight(FALSE);
            }
            break;
        }
        case VCP_IO_FUEL_LEVEL: //연료게이지 0x105
        {
            ucMsgData[0] = (pucData[0] > 100)?100:pucData[0];
            ControlFuelLevel(ucMsgData[0]);
            break;
        }
        case VCP_IO_MOTOR_SPEED: //모터 스피드 0x106
        {
            sint8 speed = pucData[0]; //0~80
            mcu_printf("[MOTOR] speed=%d\r\n", (sint8)speed);
            break;
        }
        case VCP_IO_MOTOR_WHEEL: // 서보모터 0x107
        {
            int8_t input = (int8_t)pucData[0]; // 0~256
            if (input < 0) input = 0;
            if (input > 127) input = 127;
            // 중앙 기준으로 -45~+45도로 매핑
            int angle = 90 + ((input - 65) * 45) / 65;
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
