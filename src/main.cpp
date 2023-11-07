#include <main.h>
#include <EEPROM.h>

// 0X3C+SA0 - 0x3C or 0x3D or 0x3F lcd
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1//A1

//SSD1306AsciiWire oled;
SSD1306AsciiAvrI2c oled;

Button btnUP(5,1,1,20);
Button btnDN(3,1,1,20);
Button btnSEL(4,1,1,20);

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
PID PID(&Input, &Output, &Setpoint,2,5,1,P_ON_M, DIRECT); //P_ON_M specifies that Proportional on Measurement be used
                                                            //P_ON_E (Proportional on Error) is the default behavior

//------------------------------------------------------------------------------

int tankmax=500;
float tankval=500;
float tanktotal_1=0;
int tankval_int=0;
float fuel_bar=0;
int fuel_bar_int_old=0;
int fuel_bar_int=0;
int fuel_bar_low=0;
float bar_low_perc;
int bar_low_level=50;
int roomtemp=21;

bool force=0;
byte menuselect=0;
bool blinker;
bool badread;

byte tempset_h=75; //too hot water temp
byte tempset_c=45; //too cold water temp
byte tempset_t=60; //water temp target
byte tempset_v=0; //current water temp
byte fuelrate=0; //fuel rate in hz/10 so 47= 4.7hz
byte fuelrate_h=80;
byte fuelrate_c=0;
byte fuelrate_l=22;
int fanrate_h=8200;
int fanrate_l=3000;

byte roomset=23;
bool fuelout=0;
bool kwmode=0;
byte fueluse=0;

int hours=0;
byte mins=0;

String txt_max = "MAX";
String txt_min = "MIN";

byte state=0; //state of heater

unsigned long millis_=0;

//menu

bool submenu=0;
byte menupos=0;
byte numopt=9;

//local vars

unsigned long millis_blinker=0;
unsigned long millis_second=0;
unsigned long millis_tenmin=0;
byte pumpval=22;





void setup(){

    analogReference(INTERNAL);

     //initialize the variables we're linked to
    Input = analogRead(A6);
    Setpoint= (roomset + 50) / 0.1059;

    //turn the PID on
    PID.SetMode(AUTOMATIC);

    //serial

    serialsetup();

    //all else
    
    wdt_reset();
    wdt_enable(WDTO_4S); //Enable WDT with a timeout of 2 seconds

    pinMode(A0,OUTPUT); // bright red led
    //pinMode(A1,OUTPUT);
    pinMode(A2,OUTPUT);
    digitalWrite(A2,0);

    pinMode(A6,INPUT);


    //digitalWrite(A1,0); // dim red led


    pinMode(7,OUTPUT);
    pinMode(8,OUTPUT);

    pinMode(A3,1); //voltage to temp sensor
    digitalWrite(A3,1);

    digitalWrite(8,0); //green led
    digitalWrite(7,0); //blue led


    //oled.setContrast(233);

  

      #if RST_PIN >= 0
      oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
    #else // RST_PIN >= 0
      //oled.begin(&ST7567_128x64, I2C_ADDRESS);
      oled.begin(&Adafruit128x64, I2C_ADDRESS);
    #endif // RST_PIN >= 0
      // Call oled.setI2cClock(frequency) to change from the default frequency.

    oled.setContrast(0);

    if (EEPROM.read(107)==208){ //check that an eeprom save exists
    loadEeprom();
  }
  else{
    EEPROM.write(107,208);
    writeEeprom(0);
  }

    setvals();

    initdraw();
    redraw();
    
    
}

void loop(){

  millis_=millis();

  //serialloop();

  menufn();

  if (millis_-millis_blinker >= 500){
      
      blinker=!blinker;
      millis_blinker = millis_blinker+500;

      redraw();

  }

  if (millis_-millis_second >= 1000){

      millis_second=millis_second+1000;

      tankdrop();

      cmd_send(send);
      

  }

  serialloop();

  if (millis_-millis_tenmin >= 600000L){

      millis_tenmin=millis_tenmin+600000L;

      if (state==RUN) writeEeprom(1); //write fuel level every 10 mins if running

  }

  wdt_reset();

}

unsigned long total_mins;
float drop_amt;

void tankdrop(){

    drop_amt=0;

    if (state==RUN){
      drop_amt= pumpval * fuelrate_h;
      drop_amt= drop_amt/100000.;

      tankval= max(tankval-drop_amt, 0);

      float timeleft= tankval / drop_amt;
      unsigned long timeleft_long=timeleft;

      total_mins = timeleft_long/60;

      hours= total_mins/60;
      mins=  total_mins%60;
    }
    else{
      hours= 0;
      mins=  0;
    }

    if (tankval<=0){fuelout=1;} else {fuelout=0;}

    if (kwmode){

      float kwuse= (pumpval * fuelrate);
      kwuse= kwuse * 36;
      kwuse= (kwuse / 1000)+0.5;
      fueluse= kwuse;

    }
    else{
      fueluse= fuelrate;
    }
}

void setvals(){

  cdhstat[8]=fuelrate_c;
  cdhstat[7]=fuelrate_l;
  cdhstat[5]=tempset_c;
  cdhstat[6]=tempset_h;
}

float fuelrate_smooth=fuelrate_h;

void roomtempcalc(){
  
  Input = analogRead(A6);

  float tempC = (Input * 0.1059) - 50.0;

  roomtemp = tempC;


  
  if (state!=ERROR || state!=IDLE) 
  PID.Compute();
  
  

  byte fuelHiLo_diff = fuelrate_h - fuelrate_l; // how much hz range is available

  float hz_reduct_f= (float)fuelHiLo_diff / 256; // amount of reduction over byte
        hz_reduct_f= fuelrate_h - (hz_reduct_f * (255.0-Output)); //minus the correct proportion

        //hz_reduct_f= fuelrate_h - (hz_reduct_f + 0.5); //round into integer
  

  if (fuelrate_smooth<hz_reduct_f) {fuelrate_smooth+=0.25;}
  if (fuelrate_smooth>hz_reduct_f) {fuelrate_smooth-=0.25;}

  fuelrate_c = fuelrate_smooth;

  if (tempset_v>tempset_t){  //if water is getting too hot, reduce hz by 0.2 per degree over
    fuelrate_c = fuelrate_c - ((tempset_v - tempset_t)*2);
  }

  setvals();


}

  






//EEPROM
byte writeEeprom(){
  return writeEeprom(0);
}

byte writeEeprom(byte param){

  byte nop=17;

  //menu
  noInterrupts();
#if param==0 || param==1
  float tankval_test;
  EEPROM.get(0,tankval_test);
  tankval!=tankval_test ? EEPROM.put(50,tankval) : nop--;
#endif

#if param==0 || param==2
  int tankmax_test;
  EEPROM.get(4,tankmax_test);
  tankmax!=tankmax_test ? EEPROM.put(54,tankmax) : nop--;
#endif
  interrupts();

#if param==0 || param==3
  float bar_low_perc_test;
  EEPROM.get(8,bar_low_perc_test);
  bar_low_perc!=bar_low_perc_test ? EEPROM.put(56,bar_low_perc) : nop--;
#endif

#if param==0 || param==4
  byte fuelrate_h_test;
  EEPROM.get(10,fuelrate_h_test);
  fuelrate_h!=fuelrate_h_test ? EEPROM.put(60,fuelrate_h) : nop--;
#endif

#if param==0 || param==5
  byte fuelrate_l_test;
  EEPROM.get(12,fuelrate_l_test);
  fuelrate_l!=fuelrate_l_test ? EEPROM.put(61,fuelrate_l) : nop--;
#endif

#if param==0 || param==6
  byte tempset_c_test;
  EEPROM.get(14,tempset_c_test);
  tempset_c!=tempset_c_test ? EEPROM.put(62,tempset_c) : nop--;
#endif

#if param==0 || param==7
  byte tempset_h_test;
  EEPROM.get(16,tempset_h_test);
  tempset_h!=tempset_h_test ? EEPROM.put(63,tempset_h) : nop--;
#endif

#if param==0 || param==8
  byte tempset_t_test;
  EEPROM.get(18,tempset_t_test);
  tempset_t!=tempset_t_test ? EEPROM.put(64,tempset_t) : nop--;
#endif

#if param==0 || param==9
  int fanrate_h_test;
  EEPROM.get(20,fanrate_h_test);
  fanrate_h!=fanrate_h_test ? EEPROM.put(65,fanrate_h) : nop--;
#endif

#if param==0 || param==10
  int fanrate_l_test;
  EEPROM.get(24,fanrate_l_test);
  fanrate_l!=fanrate_l_test ? EEPROM.put(67,fanrate_l) : nop--;
#endif


  return nop;

}

void loadEeprom(){


  EEPROM.get(50,tankval);

  EEPROM.get(54,tankmax);

  EEPROM.get(56,bar_low_perc);

  EEPROM.get(60,fuelrate_h);

  EEPROM.get(61,fuelrate_l);

  EEPROM.get(62,tempset_c);
 
  EEPROM.get(63,tempset_h);
 
  EEPROM.get(64,tempset_t);
 
  EEPROM.get(65,fanrate_h);
 
  EEPROM.get(67,fanrate_l);

  

}
