/*
 * Tachometer.c
 *
 * Created: 19.10.2020 18:17:09
 * Author : Gurev
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
const uint8_t numberArr[10] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6};
const uint8_t segment[7] = {0x10,0x20,0x40,0x80,0x02,0x04,0x08};
const uint16_t prescaller[5] = {1,8,64,256,1024};
uint8_t print_arr[7] = {0xfc,0x60,(0xda|0x01),0xf2,0x66,0xb6,0xbe};
volatile uint16_t captured[2];
volatile uint8_t overflowed;
static float measuredSpeed;			//Измеренная скорость
static uint8_t prescallerIndex = 0;	//По-умолчанию задаем самый быстрый прескаллер.
//PC0 - Data
//PC1 - Clock
//PC2 - Latch
//PC3 - Point
//PC4 - MODE

/*! \brief Вывод в линию.
 *
 *  Вывод на линию номера сегмента и цифры в этот сегмент.
 *
 */
void printData(void);

/*! \brief Подготовка массива для вывода данных.
 *
 *  Подаем на вход float. На выходе массив с заданным количеством
 *  знаков после запятой
 */
void prepareArray(float number, uint8_t dotPosition);
void resetTimer(void);
uint8_t maxPointPos(float number);
ISR(TIMER1_OVF_vect)			//Вектор прерывания по переполнения таймера №1
{
  TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10)); // Stop Timer1
  overflowed = 1;
}

ISR(TIMER1_CAPT_vect)			//Вектор прерывания по порту захвата таймера №1
{
  volatile uint16_t icr = ICR1;
  if (icr > captured[1]) 		//Ignore same value
  {
    captured[0] = captured[1];
    captured[1] = icr;
  }
}

int main(void)
{
	static uint8_t pointPosition = 0;
	static uint8_t rpm_mux = 1;
	DDRC = 0x07;
	PORTC = 0x18;
	DDRB &= ~(1<<8);               		//Включаем пин захвата pinMode(8, INPUT); // ICP1
  	asm volatile("cli");		  		//Выключаем прерывания
	// Настраиваем Таймер1.
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B = (1<<ICNC1); 				//Включаем детектор импульсов, считывание по ниспадающему фронту.
	TIMSK1 = (1<<ICIE1) | (1<<TOIE1); 	//Включаем прерывания по Input Capture и Overflow
    asm volatile("sei");				//Включаем прерывания
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
			prepareArray(measuredSpeed,pointPosition);
			resetTimer();
		}
		printData();
		static uint8_t last_btns = 0x18;
		switch ((PINC&0x18))
		{
		case 16:
			if (last_btns==16) break;
			pointPosition++;
			if (pointPosition>maxPointPos(measuredSpeed)) pointPosition=0;
			last_btns = 16;
			prepareArray(measuredSpeed,pointPosition);
			_delay_ms(100);
			break;
		case 8:
			if (last_btns == 8) break;
			if (rpm_mux==1)
			{
				rpm_mux=60;
				measuredSpeed*=60;
			}else
			{
				rpm_mux=1;
				measuredSpeed/=60;
			}
			last_btns = 8;
			if (maxPointPos(measuredSpeed)<pointPosition)pointPosition=maxPointPos(measuredSpeed);
			prepareArray(measuredSpeed,pointPosition);
			_delay_ms(100);
			break;
		default:
			last_btns = 24;
			break;
		}

		
	}
}


void prepareArray(float number, uint8_t dotPosition){
	uint32_t multiplexer = 1;
	for (uint8_t i = 0; i < dotPosition; i++)
	{
		multiplexer *=10;
	}
	int32_t num2Print = ((int32_t) (number*multiplexer));
	for (uint8_t i = 0; i < 7; i++)
	{
		if ((num2Print==0)&&i>dotPosition)
		{
			print_arr[i] = 0x00;
		}else
		{
			print_arr[i]= numberArr[num2Print%10];
			if (i==dotPosition) print_arr[i]|=1;			//Ставим точку где надо.
			num2Print/=10;
		}
	}
}
void printData(void){
	static uint8_t seg=0;
	uint16_t myData = print_arr[seg];
	myData <<=8;
	myData |=segment[seg];
	for (int i=0;i<16;i++)
	{
		//Задаем выходной уровень.
		if (myData&1)
		{
			PORTC |= (1<<0);
			}else {
			PORTC &= ~(1<<0);
		}
		//Посылаем строб
		PORTC |= (1<<1);
		PORTC &= ~(1<<1);
		myData>>=1;
	}
	//Передергиваем защелку
	PORTC |= (1<<2);
	PORTC &= ~(1<<2);

	seg++;
	if (seg>6)seg=0;
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

uint8_t maxPointPos(float number){
	uint8_t i;
	for (i = 7; i >0; i--)
	{
		number/=10;
		if (number<1) break;
	}
	return (i-1);
	
}
