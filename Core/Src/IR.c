#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "string.h"
#include <stdlib.h> 
#include <math.h>
#include "stdio.h"
#include "IR.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
/*
NEC红外协议是一种最常见的红外遥控协议，通常用于电视遥控器、空调遥控器等。
NEC协议通过调制红外载频信号来表示数字信号。每个数字信号由9ms的起始位和4.5ms的起始位隔开，然后由16位地址码、16位数据码和8位反码构成。
地址码用于区分不同的遥控器设备，
数据码表示遥控器键值。
接收设备在解码后，通过判断地址码和数据码来判断是哪个键被按下。

*/
#define  RX_DBG_EN   0

#define  RX_SEQ_NUM  33

#if RX_DBG_EN
#define RX_DBG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define RX_DBG(format, ...) ;
#endif

 uint8_t  tim_udt_cnt      = 0;
 uint8_t  cap_pol          = 0; 
 uint8_t  cap_pulse_cnt    = 0;
 uint8_t  sta_idle         = 1;
 uint8_t  cap_frame        = 0;
struct red_DataTypeDef red_data;
extern QueueHandle_t xqueueRedata;
extern QueueHandle_t xSemaphoreMutex;
static uint16_t rx_frame[RX_SEQ_NUM*2] = {0}; 

struct {
    uint16_t  src_data[RX_SEQ_NUM*2];
    uint16_t  repet_cnt;
    union{
        uint32_t rev;
        struct
        {
            uint32_t key_val_n:8;
            uint32_t key_val  :8;
            uint32_t addr_n   :8;
            uint32_t addr     :8;
        }_rev;
    }data;
}rx;

uint8_t appro(int num1,int num2)
{
    return (abs(num1-num2) < 300);
}

void rx_rcv_init(void)
{
    cap_frame     = 0;                                       //未捕获到新数据
    sta_idle      = 0;                                       //非空闲状态
    tim_udt_cnt   = 0;                                       //定时器溢出清0
    cap_pulse_cnt = 0;                                       //捕获到的计数清0
    
    memset(rx_frame,0x00,sizeof(rx_frame));
}

/* 溢出中断回调函数 */
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {   
// 	if(TIM1 == htim->Instance)
// 	{
//         if(sta_idle)                                          //空闲状态，不作任何处理
//         {
//             return;
//         }
        
//         tim_udt_cnt++;                                         //溢出一次
//         if(tim_udt_cnt == 3)                                   //溢出3次
//         {
//             tim_udt_cnt = 0;                                   //溢出次数清零
//             sta_idle    = 1;                                   //这是为空闲状态
//             cap_frame   = 1;                                   //标记捕获到新的数据
//         }
// 	} 
// }


/* 电平捕获中断回调 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint16_t tmp_cnt_l,tmp_cnt_h;
	if(TIM1 == htim->Instance)
	{
        switch(cap_pol)                                                                                         //根据极性标志位判断捕获是低电平还是高电平
        {   
            /* 捕获到下降沿 */
            case 0:
                tmp_cnt_l = HAL_TIM_ReadCapturedValue(&htim1,TIM_CHANNEL_1);                                    //记录当前时刻
                TIM_RESET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1);                                               //复位极性配置
                TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);                //改变极性
                
                cap_pol = 1;                                                                                    //极性标志位改为上升沿
                
                if(sta_idle)                                                                                    //如果当前为空闲状态，空闲捕获到的时序，为第一个下降沿
                {
                    rx_rcv_init();
                    break;                                                                                      //返回
                }
                rx_frame[cap_pulse_cnt] = tim_udt_cnt * 10000 + tmp_cnt_l - tmp_cnt_h;                          //与上次捕获的计时作差，记录值
                tim_udt_cnt = 0;                                                                                //溢出次数清0
                RX_DBG("(%2d)%4d us:H\r\n",cap_pulse_cnt,rx_frame[cap_pulse_cnt]);                              //DBG：打印捕获到的电平及其时长
                cap_pulse_cnt++;                                                                                //计数++
                break;
            
            /* 捕获到上升沿 */
            case 1:
                tmp_cnt_h = HAL_TIM_ReadCapturedValue(&htim1,TIM_CHANNEL_1);
                TIM_RESET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1);               
                TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
                
                cap_pol = 0;   
                if(sta_idle)
                {
                    rx_rcv_init();
                    break;
                }
                rx_frame[cap_pulse_cnt] = tim_udt_cnt * 10000 + tmp_cnt_h - tmp_cnt_l;
                tim_udt_cnt = 0;
                RX_DBG("(%2d)%4d us:L\r\n",cap_pulse_cnt,rx_frame[cap_pulse_cnt]);
                cap_pulse_cnt++;
                break;
            
            default:
                break;
        }
    }
}


/*************************************  API Layer **************************************************/

//数据解码
uint8_t hx1838_data_decode(void)
{
    memcpy(rx.src_data,rx_frame,RX_SEQ_NUM*4);
    memset(rx_frame,0x00,RX_SEQ_NUM*4);   
    RX_DBG("========= rx.src[] =================\r\n");
    for(uint8_t i = 0;i<=(RX_SEQ_NUM*2);i++)
    {
        RX_DBG("[%d]%d\r\n",i,rx.src_data[i]);
    }
    RX_DBG("========= rx.rec =================\r\n");
    if(appro(rx.src_data[0],9000) && appro(rx.src_data[1],4500))                 //#1. 检测前导码,定时器计数值是1us，所以9000就是9ms
    {
        uint8_t tmp_idx = 0;
        rx.repet_cnt  = 0;                                                       //按键重复个数清0
        for(uint8_t i = 2;i<(RX_SEQ_NUM*2);i++)                                  //#2. 检测数据
        {
            if(!appro(rx.src_data[i],560))
            {
                RX_DBG("%d,err:%d != 560\r\n",i,rx.src_data[i]);
                return 0;
            }
            i++;
            if(appro(rx.src_data[i],1680))
            {
                rx.data.rev |= (0x80000000 >> tmp_idx);                          //第 tmp_idx 为置1
                tmp_idx++;
            }
            else if(appro(rx.src_data[i],560))
            {
                rx.data.rev &= ~(0x80000000 >> tmp_idx);                         //第 tmp_idx 位清0
                tmp_idx++;
            }
            else
            {
                RX_DBG("%d,err:%d != 560||1680\r\n",i,rx.src_data[i+1]);
                return 0;
            }
        }
    }
    else if(appro(rx.src_data[0],9000) && appro(rx.src_data[1],2250) && appro(rx.src_data[2],560))
    {
        rx.repet_cnt++;
        return 2;
    }
    else
    {
        RX_DBG("前导码检测错误\r\n");
        return 0;
    }
    return 1;
}


//键值解码
void hx1838_proc(uint8_t res)
{

    if(res == 0)
    {
        return;
    }
    if(res == 2)
    {
        return;
    }

    switch(rx.data._rev.key_val)
    {
        case 162:
            // xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
            HAL_UART_Transmit(&huart1, "ceshi1\r\n", 8, HAL_MAX_DELAY);
            // xSemaphoreGive(xSemaphoreMutex);
            // memset(&red_data, 0, sizeof(red_data));
            // red_data.led_flashing_event=1;
            // xQueueSend(xqueueRedata,&red_data,100);
            break;
        case 98:
            xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
            HAL_UART_Transmit(&huart1, "2\r\n", 3, HAL_MAX_DELAY);
             xSemaphoreGive(xSemaphoreMutex);
             memset(&red_data, 0, sizeof(red_data));
            red_data.been_event=1;
            xQueueSend(xqueueRedata,&red_data,100);
            break;
        case 226:
            xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
            HAL_UART_Transmit(&huart1, "3\r\n", 3, HAL_MAX_DELAY);
             xSemaphoreGive(xSemaphoreMutex);
             memset(&red_data, 0, sizeof(red_data));
             red_data.oled_event=1;
            xQueueSend(xqueueRedata,&red_data,100);
            break;
        case 34:
            xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
            HAL_UART_Transmit(&huart1, "4\r\n", 3, HAL_MAX_DELAY);
            xSemaphoreGive(xSemaphoreMutex);
            memset(&red_data, 0, sizeof(red_data));
            red_data.all_close=1;
            xQueueSend(xqueueRedata,&red_data,100);
            break;
        case 2:
            HAL_UART_Transmit(&huart1, "5\r\n", 3, HAL_MAX_DELAY);
            break;
        case 194:
            HAL_UART_Transmit(&huart1, "6\r\n", 3, HAL_MAX_DELAY);
            break;
        case 224:
            HAL_UART_Transmit(&huart1, "7\r\n", 3, HAL_MAX_DELAY);
            break;
        case 168:
            HAL_UART_Transmit(&huart1, "8\r\n", 3, HAL_MAX_DELAY);
            break;
        case 144:
            HAL_UART_Transmit(&huart1, "9\r\n", 3, HAL_MAX_DELAY);
            break;
        case 152:
            HAL_UART_Transmit(&huart1, "0\r\n", 3, HAL_MAX_DELAY);
            break;
        case 104:
            HAL_UART_Transmit(&huart1, "*\r\n", 3, HAL_MAX_DELAY);
            break;
        case 176:
            HAL_UART_Transmit(&huart1, "#\r\n", 3, HAL_MAX_DELAY);
            break;
        case 24:
            HAL_UART_Transmit(&huart1, "^\r\n", 3, HAL_MAX_DELAY);
            break;
        case 16:
            HAL_UART_Transmit(&huart1, "<\r\n", 3, HAL_MAX_DELAY);
            break;
        case 74:
            HAL_UART_Transmit(&huart1, "v\r\n", 3, HAL_MAX_DELAY);
            break;
        case 90:
            HAL_UART_Transmit(&huart1, ">\r\n", 3, HAL_MAX_DELAY);
            break;
        case 56:
            HAL_UART_Transmit(&huart1, "ok\r\n", 4, HAL_MAX_DELAY);
            break;
        default:
            break;
    }
}



