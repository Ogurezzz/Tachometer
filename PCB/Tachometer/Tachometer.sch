EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Microchip_ATmega:ATmega328P-AU U?
U 1 1 60DCCE80
P 7800 3650
F 0 "U?" H 7800 2061 50  0000 C CNN
F 1 "ATmega328P-AU" H 7800 1970 50  0000 C CNN
F 2 "Package_QFP:TQFP-32_7x7mm_P0.8mm" H 7800 3650 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega328_P%20AVR%20MCU%20with%20picoPower%20Technology%20Data%20Sheet%2040001984A.pdf" H 7800 3650 50  0001 C CNN
	1    7800 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 60DCF5FE
P 6500 3250
F 0 "C?" H 6592 3296 50  0000 L CNN
F 1 "100nF" H 6592 3205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6500 3250 50  0001 C CNN
F 3 "~" H 6500 3250 50  0001 C CNN
	1    6500 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7800 2150 7800 1900
Wire Wire Line
	7800 1900 6500 1900
Wire Wire Line
	6500 1900 6500 3150
Wire Wire Line
	6500 3350 6500 5600
Wire Wire Line
	6500 5600 7800 5600
Wire Wire Line
	7800 5600 7800 5150
$EndSCHEMATC
