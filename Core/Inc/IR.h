/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2026-06-30 19:25:47
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2026-07-01 13:19:43
 * @FilePath: \MDK-ARMd:\Project\freertos\第五天：智能家居\智能家居\Core\Inc\IR.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __IR_H
#define __IR_H
#include "main.h"

// 完整结构体定义
struct red_DataTypeDef
{
  uint16_t led_flashing_event;
  uint16_t been_event;
  uint16_t oled_event;
  uint16_t all_close;
};

// 外部全局变量声明
extern struct red_DataTypeDef red_data;

extern uint8_t cap_frame;

uint8_t appro(int num1,int num2);
void rx_rcv_init(void);
void hx1838_cap_start(void);
void HX1838_demo(void);
void hx1838_proc(uint8_t res);
uint8_t hx1838_data_decode(void);

#endif