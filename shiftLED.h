#ifndef SHIFTLED_H
#define SHIFTLED_H

#include <avr/io.h>

#define SOFTWARE_SPI

#ifdef SOFTWARE_SPI
    #define LED_DDR         DDRC
    #define LED_PORT        PORTC
    #define LED_DAT_PIN     PORTC0
    #define LED_CLK_PIN     PORTC1
    #define LED_LAT_PIN     PORTC2
#endif

#define SEGMENTS_NUM    7           //Количество сегментов
/*! \brief Вывод в линию.
 *
 *  Вывод на линию номера сегмента и цифры в этот сегмент.
 *  @param number 
 */
void dispPrintFloat(float number);
void moveDot(float number);
void makeArr(uint32_t digit);
void printData(void);





#endif
