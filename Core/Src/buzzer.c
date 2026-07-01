/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2026-06-29 20:32:47
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2026-07-01 13:24:59
 * @FilePath: \MDK-ARMd:\Project\freertos\第五天：智能家居\智能家居\Core\Src\buzzer.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "buzzer.h"
#include "gpio.h"
 
void BUZZER_ON(void){
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
}
void BUZZER_OFF(void){
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
}