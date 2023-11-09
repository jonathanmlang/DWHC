#ifndef MAIN_H
#define MAIN_H

#include "SSD1306Ascii.h"
//#include "SSD1306AsciiWire.h"
#include "SSD1306AsciiAvrI2c.h"
#include "Button.h"
#include <avr/wdt.h>
#include <PID_v1.h>

extern SSD1306AsciiAvrI2c oled;

extern Button btnUP;
extern Button btnDN;
extern Button btnSEL;

#define  IDLE     0
#define  PREHEAT  1
#define  RUN      3
#define  ERROR    5
#define  COOL     4
#define  NONE     6


extern double Setpoint, Input, Output;

extern int tankmax;
extern float tankval;
extern float tanktotal_1;
extern int tankval_int;
extern float fuel_bar;
extern int fuel_bar_int_old;
extern int fuel_bar_int;

extern int fuel_bar_int;
extern int fuel_bar_low;
extern float bar_low_perc;
extern int bar_low_level;

extern bool force;
extern byte menuselect;
extern bool blinker;

extern byte tempset_h;
extern byte tempset_c;
extern byte tempset_t;
extern byte tempset_v;
extern byte fuelrate;

extern byte roomset;
extern byte fuelrate_h;
extern byte fuelrate_c;
extern byte fuelrate_l;
extern int fanrate_h;
extern int fanrate_l;
extern int fanrate_c;
extern bool kwmode;
extern byte fueluse;

extern String txt_max;
extern String txt_min;

//menu
extern bool submenu;
extern byte menupos;
extern byte numopt;
extern byte state;
extern bool glow;
extern bool fuelout;

extern int hours;
extern byte mins;
extern unsigned long total_mins;
extern float drop_amt;

extern byte cdhstat[37];

extern int roomtemp;

extern int fan_speed;

//serial

#define  STAT   3
#define  ON     1
#define  OFF    2
#define  U_PUMP 3

extern byte send;

extern float fuelrate_c_f;

extern bool badread;

byte writeEeprom();
byte writeEeprom(byte);
void loadEeprom();

void tankdrop();

void initdraw();
void drawbarfuel();
void drawfueltext();
void drawtemp();
void printstate(byte);
void drawstate();
void drawfuelrate();
void drawroomtemp();
void drawfueltime();
void redraw();
void setvals();
void roomtempcalc();

void menufn();

void serialsetup();
void serialloop();
void setCRC();
void setCRCa(uint16_t);
void setCRCb(uint16_t);
void cmd_send(byte);




#endif