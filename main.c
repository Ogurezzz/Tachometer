/*
 * Tachometer.c
 *
 * Created: 19.10.2020 18:17:09
 * Author : Gurev
 */ 

#include <avr/io.h>
#include <util/delay.h>
const uint8_t numberArr[10] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6};
const uint8_t segment[7] = {0x10,0x20,0x40,0x80,0x02,0x04,0x08};
uint8_t print_arr[7] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe};


//PC0 - Data
//PC1 - Clock
//PC2 - Latch
//PC3 - Point
//PC4 - MODE
void printData(uint8_t digit, uint8_t segmentNum);
void printArr(void);




int main(void)
{
	DDRC = 0b00000111;
	PORTC = 0;
    
    while (1) 
    {
		printArr();
	}
}

void printData(uint8_t digit, uint8_t segmentNum){
	uint16_t myData = digit;
	myData <<= 8;
	myData |= segmentNum;
	
	for (int i=0;i<16;i++){
		//Задаем выходной уровень.
		if (myData&1) {
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
}
void printArr(void){
	for (uint8_t i=0;i<7;i++){
		printData(print_arr[i],segment[i]);
	}
}