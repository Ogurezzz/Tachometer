# Tachometer
Simple tachometer, based on arduino nano.
7 digit display based on ULN2003 and 74HC595
3 buttons:
1. Reset
2. Mode switch. Between Hz and RPM.
3. Point position. Chose how many digits after point will be displayed.

This tachometer uses ATmega328p Input capture register of Timer Counter #1.
Also I used a lot of float math in this code. ToDo is to move all math into static point to improve speed.
All Schematics and PCB you can find here: https://easyeda.com/gurevoleg/tachometer