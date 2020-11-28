/*
 * Tachometer.c
 *
 * Created: 19.10.2020 18:17:09
 * Author : Gurev
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "shiftLED.h"
const uint16_t prescaller[5] = {1,8,64,256,1024};

volatile uint16_t captured[2];
volatile uint8_t overflowed;
float measuredSpeed;			//Измеренная скорость
uint8_t prescallerIndex = 0;	//По-умолчанию задаем самый быстрый прескаллер.
uint8_t rpm_mux = 1;

#define BTN_MASK 0x18		//MASK = MODE|POINT
#define BTN_MODE 0x08		//PC4 - MODE
#define BTN_POINT 0x10		//PC3 - POINT


//void printData(void);

/*! \brief Подготовка массива для вывода данных.
 *
 *  Подаем на вход float. На выходе массив с заданным количеством
 *  знаков после запятой
 */
void resetTimer(void);
void doTick(void);
void keyboardAction(void);

ISR(TIMER1_OVF_vect)			//Вектор прерывания по переполнения таймера №1
{
  TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10)); // Stop Timer1
  overflowed = 1;
}

ISR(TIMER1_CAPT_vect)			//Вектор прерывания по порту захвата таймера №1
{
  volatile uint16_t icr = ICR1;
  if (icr > captured[1]) 		// новое должно быть больше старого.
  {
    captured[0] = captured[1];
    captured[1] = icr;
  }
}

ISR(TIMER2_COMPA_vect)								//Прерывание. 1 раз в мс.
{
	doTick();
}

int main(void)
{
	DDRC = 0x07;
	PORTC = 0x18;
	DDRB &= ~(1<<8);               		//Включаем пин захвата
  	asm volatile("cli");		  		//Выключаем прерывания

	// Настраиваем Таймер1.
	TCCR1A = 0;
	TCCR1B = (1<<ICNC1); 				//Включаем детектор импульсов, считывание по ниспадающему фронту.
	TIMSK1 = (1<<ICIE1) | (1<<TOIE1); 	//Включаем прерывания по Input Capture и Overflow

	//Настраиваем таймр тиков (1мс)
	TCCR2A = 2<<WGM20; 				// Установить режим Compare Match
    TCCR2B = 4<<CS20;               // Предделитель 64
	OCR2A  = 125; 					// Установить значение в регистр сравнения
	TCNT2 = 0;						// Установить начальное значение счётчиков
	TIMSK2 |= 1<<OCIE2A;			// Разрешаем прерывание RTOS - запуск ОС
    asm volatile("sei");			//Включаем прерывания
	resetTimer();
    while (1) 
    {
		if (overflowed)
		{
			
			if (captured[0] && captured[1]) 
			{
				measuredSpeed = (((float)((F_CPU*rpm_mux) / prescaller[prescallerIndex])) / (captured[1] - captured[0]));
				if ( (uint32_t) (captured[1] - captured[0])*(prescaller[prescallerIndex]/prescaller[prescallerIndex-1]) < 65535) 
				{
					if (prescallerIndex>0) prescallerIndex--;
				}
			}else if(!captured[1])
			{
				if (prescallerIndex<3)prescallerIndex++;
				measuredSpeed = 0;
			}
			dispPrintFloat(measuredSpeed);
			resetTimer();
		}
	}
}


void resetTimer(void){
		asm volatile("cli");		  		//Выключаем прерывания
		//Обнуляемся, настраиваем предделитель и прочая лабуда.
		overflowed = 0;	//Сбрасываем флаг переполнения таймера.
		captured[0] = captured[1] = 0;		//Обнуляем значения стробов.
		TCNT1 = 0;							//Обнуляем значение таймера
		//Выбор и установка прескаллера.
		switch (prescaller[prescallerIndex]){
			case 1:
			TCCR1B = 0x81;
			break;
			case 8:
			TCCR1B = 0x82;
			break;
			case 64:
			TCCR1B = 0x83;
			break;
			case 256:
			TCCR1B = 0x84;
			break;
			case 1024:
			TCCR1B = 0x85;
			break;
		}
		TIFR1 = (0<<ICF1) | (0<<TOV1);      // Обнуляем прерывания, если вдруг они случились, пока мы тут тусим.
    	asm volatile("sei");				//Включаем прерывания
}

void doTick(void){
	static uint8_t ticks = 0;
	printData();
	ticks++;
	if (ticks>100){
		keyboardAction();
		ticks = 0;
	}
}

void keyboardAction(void){
	static uint8_t last_btns = 0x18;
	uint8_t currButtons = PINC&BTN_MASK;
	if (currButtons==last_btns)return;
		switch ((currButtons))
		{
		case BTN_POINT:
			if (last_btns==16) break;
			moveDot(measuredSpeed);
			break;
		case BTN_MODE:
			if (last_btns == 8) break;
			if (rpm_mux == 1)
			{
				rpm_mux=60;
				measuredSpeed*=60;
			}else
			{
				rpm_mux=1;
				measuredSpeed/=60;
			}
			dispPrintFloat(measuredSpeed);
			break;
		default:
			break;
		}
	last_btns = currButtons;
}