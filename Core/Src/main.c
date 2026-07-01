/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "lightADC.h"
#include "IR.h"
#include "CurtainSwitch.h"
#include "DHT11.h"
#include "buzzer.h"
#include <stdio.h>
#include <string.h>
#include "semphr.h"
#include "event_groups.h"
#include <stdlib.h> 
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define EVENT_LED_FLASH  (1 << 0)
#define EVENT_BEEP       (1 << 1)
#define EVENT_OLED       (1 << 2)
#define EVENT_CLOSE      (1 << 3)
SemaphoreHandle_t xSemaphore;
EventGroupHandle_t xEventgroup;
QueueHandle_t xSemaphoreMutex;
QueueHandle_t xqueueRedata;  // 红外数据接收队列
EventGroupHandle_t xEventGroupRedTask;

TaskHandle_t start_task_handle;
TaskHandle_t light_task_handle;
TaskHandle_t temp_task_handle;
TaskHandle_t ultrasound_task_handle;
TaskHandle_t rtcTime_task_handle;
TaskHandle_t pwmLed_task_handle;
TaskHandle_t red_task_handle;
TaskHandle_t  event_dispatch_task_handle;
TaskHandle_t flashing_task_handle;


void start_task(void *pvParameters);
void light_task(void *pvParameters);
void temp_task(void *pvParameters);
void pwmLed_task(void *pvParameters);
void ultrasound_task(void *pvParameters);
void rtcTime_task(void *pvParameters);
void red_task(void *pvParameters);
void event_dispatch_task(void *pvParameters);
void flashing_task(void *pvParameters);

#define EVENT_LED (1<<0)
// ??????????????
#define REMOTE_ID 0x00

// ????????????
uint8_t RmtSta = 0;
uint16_t Dval;
uint32_t RmtRec = 0;
uint8_t RmtCnt = 0;

extern uint8_t tim_udt_cnt;
extern uint8_t cap_pol;
extern uint8_t cap_pulse_cnt;
extern uint8_t sta_idle;
extern uint8_t cap_frame;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void switch_num(uint16_t num);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
// PA0 ?????????500ms??????

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,0);

    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_IC_Start_IT(&htim1,TIM_CHANNEL_1);
// HAL_TIM_IC_Start_IT(&htim1,TIM_CHANNEL_2);
// __HAL_TIM_SET_CAPTUREPOLARITY(&htim1,TIM_CHANNEL_2,TIM_ICPOLARITY_FALLING);
// TIM1->CNT=0;
// ????RTC??
RTC_TimeTypeDef timeCfg = {0};
timeCfg.Hours = 22;    // ?? 0~23
timeCfg.Minutes = 0;  // ?? 0~59
timeCfg.Seconds = 0;  // ? 0~59
HAL_RTC_SetTime(&hrtc, &timeCfg, RTC_FORMAT_BIN);

RTC_DateTypeDef dateCfg = {0};
dateCfg.WeekDay = RTC_WEEKDAY_TUESDAY; // ??
dateCfg.Month = RTC_MONTH_JUNE;        // ??
dateCfg.Date = 30;                     // ??
dateCfg.Year = 26;                     // ?????2026
HAL_RTC_SetDate(&hrtc, &dateCfg, RTC_FORMAT_BIN);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  // ???????????¨??????
  BaseType_t createRet = xTaskCreate(start_task, "start_task", 128, NULL, 2, &start_task_handle);
  if(createRet != pdPASS)
  {
    while(1)
    {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
      HAL_Delay(300);
    }
  }

  vTaskStartScheduler();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void start_task(void *pvParameters)
{
  char buff[32];
  xSemaphore = xSemaphoreCreateCounting( 1, 1 );
  xEventgroup=xEventGroupCreate();
  //   if(xEventgroup != NULL)
  // {
  //     xEventGroupClearBits(xEventgroup, EVENT_LED);
  // }
    xqueueRedata=xQueueCreate(3,sizeof(struct red_DataTypeDef));
     xEventGroupRedTask=xEventGroupCreate();
  xSemaphoreMutex= xSemaphoreCreateMutex();
  if(xSemaphore !=NULL &&xEventgroup!=NULL){
    sprintf(buff,"success\r\n");
    xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
    HAL_UART_Transmit(&huart1,(uint8_t *)buff, strlen(buff),100);
    xSemaphoreGive(xSemaphoreMutex);
  }
 
  taskENTER_CRITICAL();
  xTaskCreate(light_task, "light_task", 128, NULL, 2, &light_task_handle);
  xTaskCreate(temp_task, "temp_task", 128, NULL, 1, &temp_task_handle);
  xTaskCreate(pwmLed_task, "pwmLed_task", 128, NULL, 4, &pwmLed_task_handle);
  xTaskCreate(ultrasound_task, "ultrasound_task", 128, NULL, 3, &ultrasound_task_handle);
  xTaskCreate(rtcTime_task, "rtcTime_task", 128, NULL, 3, &rtcTime_task_handle);
  xTaskCreate(red_task, "red_task", 128, NULL, 3, &red_task_handle);
    xTaskCreate(event_dispatch_task,"event_dispatch_task",256,NULL,3,&event_dispatch_task_handle);
     xTaskCreate(flashing_task, "flashing_task", 256, NULL, 2, &flashing_task_handle);
  taskEXIT_CRITICAL();
  vTaskDelete(NULL); 
}

void flashing_task(void *pvParameters){
   static uint8_t flash_enable = 0;
  while(1){
    EventBits_t bits= xEventGroupWaitBits(xEventGroupRedTask,EVENT_LED_FLASH | EVENT_CLOSE,pdTRUE,pdFALSE,100);
    // if(bits&EVENT_LED_FLASH){
    //   // HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
    //    for(uint8_t i = 0; i < 100; i++)
    //         {
    //           HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
    //           vTaskDelay(pdMS_TO_TICKS(100));
    //           HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
    //           vTaskDelay(pdMS_TO_TICKS(100));
    //   //           HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    //   //           vTaskDelay(pdMS_TO_TICKS(300));
    //         }
       
    //   //  BUZZER_ON();
    //   //           vTaskDelay(pdMS_TO_TICKS(500));
    //   //           BUZZER_OFF();
    //   }
    //    if(bits&EVENT_CLOSE){
    //     HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
    //   }
     if(bits & EVENT_LED_FLASH){
            flash_enable = 1; // ??????
        }
        if(bits & EVENT_CLOSE){
            flash_enable = 0;
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
        }

        // ???????????????
        if(flash_enable){
            HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
        }else{
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
        }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// extern uint8_t cap_frame;
 void event_dispatch_task(void *pvParameters){
  while(1){
    // struct red_DataTypeDef data;
   if(xQueueReceive(xqueueRedata,&red_data,100)==pdPASS){
      if(red_data.been_event==1){
        xEventGroupSetBits(xEventGroupRedTask,EVENT_BEEP);
      }else if(red_data.led_flashing_event==1){
        xEventGroupSetBits(xEventGroupRedTask,EVENT_LED_FLASH);
      }else if(red_data.oled_event==1){
        xEventGroupSetBits(xEventGroupRedTask,EVENT_LED);
      }else if(red_data.all_close==1){
        xEventGroupSetBits(xEventGroupRedTask,EVENT_CLOSE);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  
 }

void red_task(void *pvParameters){
  while (1)
  { 
        if(cap_frame)//?????????
        {   
            HAL_UART_Transmit(&huart1,"Decode Start\r\n",14,100);
            hx1838_proc(hx1838_data_decode());//????
            cap_frame = 0;
						
		
        }

    // 50ms????????CPU
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void light_task(void *pvParameters)
{
  uint16_t value=0;
  char buff[32];
  while(1){
    value=Read_light_value();
    sprintf(buff,"light value:%d\r\n",value);
    xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
    HAL_UART_Transmit(&huart1,(uint8_t *)buff, strlen(buff),100);
    xSemaphoreGive(xSemaphoreMutex);
    vTaskDelay(1000);
    if(value<300){
      switch_num(50);
    }else if(value>=300&&value<1000){
       switch_num(250);
    }
  }
}

void temp_task(void *pvParameters)
{
 float value;
  char buff[32];
  DHT11_Data tempdat;
  while(1){
    if(DHT11_Write()==DHT11_OK){
      if( DHT11_Read(&tempdat)==DHT11_OK){
                sprintf(buff,"Temperature: %.1f C, Humidity: %.1f%%RH\r\n", 
                tempdat.temperature, tempdat.humidity);
        xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
        HAL_UART_Transmit(&huart1,(uint8_t *)buff, strlen(buff),100);
        xSemaphoreGive(xSemaphoreMutex);
      }
     
    }
    if(tempdat.temperature<31){
      // switch_num(250);
      BUZZER_OFF();
    }else if(tempdat.temperature>=31){
      //  switch_num(50);
       BUZZER_ON();
    }
    vTaskDelay(1000);
  }
}

void rtcTime_task(void *pvParameters){
  RTC_TimeTypeDef stime;
  RTC_DateTypeDef sData;
  char buff[32];
  while(1){
   
    HAL_RTC_GetTime(&hrtc,&stime,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&sData,RTC_FORMAT_BIN);
    if(stime.Hours>=21||stime.Hours<=8){
      xEventGroupSetBits(xEventgroup,EVENT_LED);
    }else{
      xEventGroupClearBits(xEventgroup,EVENT_LED);
    }
    sprintf(buff,"time:%d\r\n",stime.Hours);
    xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
    HAL_UART_Transmit(&huart1,(uint8_t *)buff,strlen(buff),100);
    xSemaphoreGive(xSemaphoreMutex);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void pwmLed_task(void *pvParameters){
  while (1)
  {
    // EventBits_t bits = xEventGroupGetBits(xEventgroup);
    EventBits_t bits =xEventGroupWaitBits(xEventgroup,EVENT_LED,pdTRUE,pdFALSE,portMAX_DELAY);
    if(bits & EVENT_LED){
      // HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
      for(uint32_t i=0;i<1999;i++){
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,i);
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      for(uint32_t i=1999;i>0;i--){
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,i);
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      // HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2);

    }else{
      __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,0);
    }
  }
}
void ultrasound_task(void *pvParameters){
  char buff[32];
   HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim2,TIM_CHANNEL_2);
  while(1){
    // ??cut
    __HAL_TIM_SET_COUNTER(&htim2,0);
    // ??cc1?cc2??
    __HAL_TIM_CLEAR_FLAG(&htim2,TIM_FLAG_CC1);
    __HAL_TIM_CLEAR_FLAG(&htim2,TIM_FLAG_CC2);
    // ??1?2??
   
    // trig??10us ?????????
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);
   vTaskDelay(pdMS_TO_TICKS(1)); 
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);
    // ?38us???????????50???????
    // HAL_GetTick()???????
    // uint16_t expireTime=HAL_GetTick()+50;
    uint16_t expireTime=xTaskGetTickCount() + pdMS_TO_TICKS(50);
    // ????? 1?????
    uint8_t success=0;
    while(expireTime>xTaskGetTickCount()){
      uint32_t cc1Flag=__HAL_TIM_GET_FLAG(&htim2,TIM_FLAG_CC1);
      uint32_t cc2Flag=__HAL_TIM_GET_FLAG(&htim2,TIM_FLAG_CC2);
      if(cc1Flag&&cc2Flag){
         success=1;
         break;
      }
    }
    // ????
    // HAL_TIM_IC_Stop(&htim2,TIM_CHANNEL_1);
    // HAL_TIM_IC_Stop(&htim2,TIM_CHANNEL_2);
    if(success){
       // ???? 1e-6f??1us,1us=1/72mhz??71+1?
      uint32_t cc1value=__HAL_TIM_GET_COMPARE(&htim2,TIM_CHANNEL_1 );
      uint32_t cc2value=__HAL_TIM_GET_COMPARE(&htim2,TIM_CHANNEL_2);
      // ??
      float pulswidth=(cc2value-cc1value)*1e-6f;
      // ??=??*?????2 ????=??
      float distance=340.0f*pulswidth/2.0f;
        sprintf(buff,"distance: %.1f\r\n", 
                distance);
      xSemaphoreTake(xSemaphoreMutex,portMAX_DELAY);
      HAL_UART_Transmit(&huart1,(uint8_t *)buff, strlen(buff),100);
      xSemaphoreGive(xSemaphoreMutex);
      if(distance <0.1){
        BUZZER_ON();
      }else{
        BUZZER_OFF();
      }
    }
    // else
    //   {
    //       sprintf(buff, "Timeout: No echo received!\r\n");
    //       HAL_UART_Transmit(&huart1, (uint8_t *)buff, strlen(buff), 100);
    //   }
   

vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void switch_num(uint16_t num){
  if(xSemaphoreTake(xSemaphore,pdMS_TO_TICKS(100))==pdTRUE){
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,num);
     vTaskDelay(pdMS_TO_TICKS(500)); 
     xSemaphoreGive(xSemaphore);
  }
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	if(TIM1 == htim->Instance)
	{
        if(sta_idle)                                          //空闲状态，不作任何处理
        {
            return;
        }
        
        tim_udt_cnt++;                                         //溢出一次
        if(tim_udt_cnt == 3)                                   //溢出3次
        {
            tim_udt_cnt = 0;                                   //溢出次数清零
            sta_idle    = 1;                                   //这是为空闲状态
            cap_frame   = 1;                                   //标记捕获到新的数据
        }
	} 
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
