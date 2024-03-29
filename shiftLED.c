#include "shiftLED.h"
const uint8_t numberArr[10] PROGMEM = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6};
const uint8_t segment[7] PROGMEM = {0x10,0x20,0x40,0x80,0x02,0x04,0x08};
uint8_t screenArr[SEGMENTS_NUM*2];
uint8_t dotPosition = 2;


void moveDot(float number){
    dotPosition++;
    if (dotPosition>=SEGMENTS_NUM)dotPosition=0;
    dispPrintFloat(number);
}

void dispPrintFloat(float number){
    uint8_t maxDotPosition = maxPointPos(number);
	uint32_t multiplexer = 1;
    if (dotPosition>maxDotPosition) dotPosition=maxDotPosition;
	for (uint8_t i = 0; i < dotPosition; i++){
		multiplexer *=10;
	}
    makeArr(number*multiplexer);
}
void dispPrintInt(uint32_t number){
	dotPosition=2;
	makeArr(number);
}

void makeArr(uint32_t digit){
uint8_t *outArr = &screenArr[0];
for (uint8_t i = 0; i < SEGMENTS_NUM; i++)
	{
            *outArr= pgm_read_byte(&(segment[i]));
            outArr++;
            if ((digit==0)&&i>dotPosition)
            {
                *outArr = 0x00;
            }else
            {
                *outArr= pgm_read_byte(&(numberArr[digit%10]));
                if (i==dotPosition) *outArr|=1;			//Ставим точку где надо.
                digit/=10;
            }
            outArr++;
	}
}

void printData(){
    static uint8_t seg = 0;
    uint8_t myData = screenArr[seg];
	for (int i=0;i<8;i++)
	{
		//Задаем выходной уровень.
		if (myData&1)
		{
			LED_PORT |= (1<<LED_DAT_PIN);
			}else {
			LED_PORT &= ~(1<<LED_DAT_PIN);
		}
		//Посылаем строб
		LED_PORT |= (1<<LED_CLK_PIN);
		LED_PORT &= ~(1<<LED_CLK_PIN);
		myData>>=1;
	}
	//Передергиваем защелку на каждом втором такте
    if (seg%2)
    {
        LED_PORT |= (1<<LED_LAT_PIN);
        LED_PORT &= ~(1<<LED_LAT_PIN);
    }
	seg++;
	if (seg==(SEGMENTS_NUM*2))seg=0;
    
}

uint8_t maxPointPos(float number){
	uint8_t i;
	for (i = SEGMENTS_NUM; i >0; i--)
	{
		number/=10;
		if (number<1) break;
	}
	return (i-1);
	
}

