#include "lightADC.h"
#include "adc.h"



uint16_t Read_light_value(void){
     uint16_t value = 0; 
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1,100)==HAL_OK){
       value=HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return value;
}

