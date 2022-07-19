/*
 * Tachometer.c
 *
 * Created: 19.10.2020 18:17:09
 * Author : Gurev
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include "shiftLED.h"

#define BUF_SIZE	16
#define BTN_MASK 	0x18		//MASK = MODE|POINT
#define BTN_MODE 	0x08		//PC4 - MODE
#define BTN_POINT 	0x10		//PC3 - POINT
#define F_CPU2		16021323UL


volatile float 	measuredSpeed;				//Измеренная скорость
volatile uint32_t 	cycles = 0;				//Количество полных циклов с последнего отсчета
volatile uint8_t 	rpm_mux = 60;			//Множитель скорости
volatile float 	measureBuff[BUF_SIZE];		//Буффер с измерениями.
volatile uint8_t	measuredIndex=0;		//Индекс буффера.


void doTick(void);						//Рутина на каждую 1мс
void keyboardAction(void);				//Проверка клавиатуры



ISR(TIMER1_OVF_vect,ISR_NOBLOCK)			//Вектор прерывания по переполнения таймера №1
{
	cycles++;
}

ISR(TIMER1_CAPT_vect)						//Вектор прерывания по порту захвата таймера №1
{
	volatile uint32_t delta=0;
	TCNT1=0;
	delta = ICR1;
	delta+= ((cycles)*0xffff);
	cycles = 0;
	measureBuff[measuredIndex++] = ((float)F_CPU2/ (float)delta);
	if (measuredIndex==BUF_SIZE)	measuredIndex = 0;
}

ISR(TIMER2_COMPA_vect,ISR_NOBLOCK)								//Прерывание. 1 раз в мс. Без блокировки остальных прерываний
{
	doTick();
}

int main(void)
{
	DDRC = 0x07;
	PORTC = 0x18;
	DDRB &= ~_BV(PB0);               	//Включаем пин захвата
	PORTB |= _BV(PB0);					//Внутренняя подтяжка
  	asm volatile("cli");		  		//Выключаем прерывания

	// Настраиваем Таймер1.
	TCCR1A = 0;
	TCCR1B = _BV(ICNC1)|_BV(ICES1)|_BV(CS10); 		//Включаем детектор импульсов, считывание по возрастающему фронту.
	TIMSK1 = _BV(ICIE1)|_BV(TOIE1); 				//Включаем прерывания по Input Capture и Overflow

	//Настраиваем таймр тиков (1мс)
	TCCR2A = 2<<WGM20; 				// Установить режим Compare Match
    TCCR2B = 4<<CS20;               // Предделитель 64
	OCR2A  = 250; 					// Установить значение в регистр сравнения
	TCNT2 = 0;						// Установить начальное значение счётчиков
	TIMSK2 |= 1<<OCIE2A;			// Разрешаем прерывание RTOS - запуск ОС
    asm volatile("sei");			//Включаем прерывания
    while (1) 
    {

	}
}

void doTick(void){
	static uint8_t kbdTicks = 0;
	static uint16_t dispTicks = 0;

		printData();

	if (kbdTicks>50){
		keyboardAction();
		kbdTicks = 0;
	}else{
		kbdTicks++;
	}

	if (dispTicks>1000){
		double calcSpeed = 0;
		int8_t i = 0;
		if (measuredIndex){
			i=measuredIndex-1;
		}else{
			i=BUF_SIZE-1;
		}
		if (measureBuff[i]>2){
			for (uint8_t j = 0; j<BUF_SIZE;j++){
				calcSpeed+=(measureBuff[j]);
			}
			calcSpeed/=BUF_SIZE;
		}else{
			calcSpeed=measureBuff[i];
		}
		dispPrintFloat((float) (calcSpeed*rpm_mux));
		
		dispTicks = 0;
	}else{
		dispTicks++;
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
				rpm_mux = 60;
			}else
			{
				rpm_mux = 1;
			}
				
			break;
		default:
			break;
		}
	last_btns = currButtons;
}

