This is a VERY EXPERIMENTAL and early days LCD or OLED based controller for chinese diesel water heaters. Its slow and full of bugs and ugly code!
It allows for PID control, estimated fuel gauge and time remaining until empty.
It also contains information on how to decode the obfuscated serial protocol detailed in serial.cpp

To run this you need an ATMEGA328P based arduino or alike board

Currently supports many oled i2c displays like SSD1306 and the ST7567 lcd.

Wiring:
Oled/lcd to the i2c pins A4/A5
Up button pin 5
Select button pin 4
Down button pin 3

Blue serial wire pin 12

Red led pin A0
Green led pin 8
Blue led pin 7

TMP36 temperature sensor vout pin A6
