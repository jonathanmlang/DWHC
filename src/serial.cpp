#include <main.h>

void serialsetup();
void serialloop();
void setCRC();
void setCRCa(uint16_t);
void setCRCb(uint16_t);
void cmd_send(byte);
void checkCRC();
void setCRCc(uint16_t);


//-------------------

#include <SoftwareSerialWithHalfDuplex.h>

#include <MODBUS-CRC16.h>

#define comPin 11//6

SoftwareSerialWithHalfDuplex cdh(comPin,comPin);

char inChar=0;
String inputString="";
bool wasread=1;

bool wascmd=0;
bool newline=1;
bool decimal=0;
byte crc1;
byte crc2;
bool snd=0;

bool glow=0;

byte minusbyte=0;
byte cdhon_hash[37];




#define  STAT   3
#define  ON     1
#define  OFF    2
#define  U_PUMP 3

byte send=STAT;


/*
Transmit to heater byte structure

1,2	          Controller or mainboard
3	            Off or on command 5B=on 5B=off 01=nop, 
25,26,35,36	  CRC16 MODBUS of all vals to the left
4	            5B to power ON and XOR all values with this to unscramble left of the red line
27	          XOR all values with this to unscramble right of the red line
17	          08 FU-1 0B FU-2 and L value, dont know what this does *
6	            L-79 Temp
7	            H-94 Temp
8	            Fuel Pump low speed 1.8Hz
9	            Fuel Pump high speed 6.0Hz
10,11	        Impeller low MSB - LSB 3000rpm last hex numbers is +1'ed *
12,13	        Impeller high MSB - LSB 8200rpm last hex is +1'ed *
	            ? *
28-34         useless *
35,36         Unknown, Not crc but needed to keep heater on *


Response from heater byte structure

1,2	          Controller or mainboard
3	            On or off
35,36	        CRC16 MODBUS of all vals to the left
4	            XOR all values with this to unscramble all
5,6	          State, col 5 when warmup and col 6 when at full power, xor vals with col 5 to unscramble
7	            input voltage as actual
26	          water temp 1
11	          MSB of actual fan rpm *2 due to 2 magnets
9	            LSB of actual fan rpm *2 due to 2 magnets
10,12-14,19,23,25,27-34   	useless
20	          water temp 2 part scrambled
22,24	        unknown
17	          02 pump off, speed of pump when on
8	            fan icon speed only


Some of these may be wrong, more testing needed * means not confident

Serial baud is 7845 as verified by a scope.

*/



//vars
byte in_bytes[40] =
// 1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24  c25  c26  d27  u28  u29  u30  u31  u32  u33  u34  c35  c36
{0xA6,0x18,0x03,0x01,0x00,0x4E,0x55,0x13,0x3D,0x0A,0xB9,0x21,0x63,0x79,0x04,0x00,0x08,0x29,0x01,0xC9,0x00,0x01,0x01,0x19,0x61,0x1C,0x28,0x00,0x00,0x00,0x00,0x04,0x17,0x6A,0x27,0xD2};

uint8_t cdhstat[37] =
// 1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24  c25  c26  d27  u28  u29  u30  u31  u32  u33  u34  c35  c36
{0xA6,0x18,0x03,0x01,0x00,0x4E,0x55,0x13,0x3D,0x0A,0xB9,0x21,0x09,0x79,0x04,0x00,0x08,0x29,0x01,0xC9,0x00,0x01,0x01,0x19,0x00,0x00,0x28,0x00,0x00,0x00,0x00,0x02,0xFD,0x6A,0x23,0x00};


//byte fuelrate=0;
//byte state=0; //state of heater
//byte tempset_v=0; //current water temp


void serialsetup(){

  cdh.begin(7485);
  
  //Serial.println("Ready");



}

byte nosig=0;
int serial_wait=0;
void serialloop(){

    int n=0;


  //while (!cdh.available() &&  ++serial_wait < 5000){}

  serial_wait=0;

   
  while (cdh.available() && n<40) {
    
    wasread=1;
    nosig=0;
    
    in_bytes[n] = cdh.read(); 
        n++;
    delay(2);
 }


    
       
   

byte printbyte[37]= {0};
 if (wasread==1){

  if (in_bytes[0]==0x6A){
    crc1= in_bytes[34];
    crc2= in_bytes[35];
    

    checkCRC();
    
    if (cdhon_hash[34]==crc1 && cdhon_hash[35]==crc2){

        badread=0;

        for (byte st=0; st<36; st++){

        
              if (in_bytes[0]==0x6A){
                  printbyte[st]= (in_bytes[3] ^ in_bytes[st]);// - makezero;
              }

            } 

            if (in_bytes[0]==0x6A){

                fuelrate= printbyte[16] ^ printbyte[5];
                

                bool validstate=0;
                if (printbyte[2]== 0x02 && printbyte[4]==0x00){validstate=1; state=IDLE;} //off
                if (printbyte[2]== 0x03 && printbyte[4]==0x01){validstate=1; state=PREHEAT;} //preheat glowoff
                if (printbyte[2]== 0x00 && printbyte[4]==0x03){validstate=1; state=PREHEAT;} //preheat glowon
                if (printbyte[2]== 0x06 && printbyte[4]==0x05){validstate=1; state=PREHEAT;} //preheat glowoff
                if (printbyte[2]== 0x07 && printbyte[4]==0x04){validstate=1; state=RUN;} //run at temp
                if (printbyte[2]== 0x05 && printbyte[4]==0x06){validstate=1; state=COOL;} //cool
                if (validstate==0) {state=ERROR;}

                if ((printbyte[5]>1 && printbyte[5]<4) || (printbyte[5]>5 && printbyte[13]<7))
                    {glow=1;} else {glow=0;}

                byte watertemp= printbyte[19] ^ printbyte[5]; // water temperature is at position 19

                tempset_v= watertemp;

                int fan;
                byte fana= printbyte[10] ^ printbyte[5];
                byte fanb= printbyte[8] ^ printbyte[5];;
                fan = (fana<<8) | fanb; // remember to dvide by 2 because 2 magnets
                
              } //for (byte st=0; st<36; st++)

          } //if crc test
          else{
            badread=1;
          }
  }  //if (in_bytes[0]==0x6A)
  }  //if (wasread==1)

 wasread=0;

 for(byte pos=0; pos<40; pos++){
      in_bytes[pos] = 0;
    }

  if (send!=ON || send!=OFF) send=STAT;
  

    
}

void cmd_send(byte cmd){

  cdhon_hash[0]=cdhstat[0];
  cdhon_hash[1]=cdhstat[1];

  if (fuelout && (state!=COOL || state!=IDLE ) && cmd==3) {cmd=2;} //turn off and lock off
    
    for (byte dehash=2; dehash<34; dehash++){

      if (dehash==3){
        if (cmd==3) cdhstat[dehash] = 0x01; //stat
        if (cmd==1) cdhstat[dehash] = 0x5B; //on
        if (cmd==2) cdhstat[dehash] = 0x5D; //off
      }
      cdhon_hash[dehash] = cdhstat[dehash];// ^ (in_bytes[3]+1);
    }
    //while (cdh.available()){}
    
    snd=1;
    setCRC();
    snd=0;

    cdhon_hash[34]=in_bytes[34];

    cdh.write(cdhon_hash,37);
    
}


//CRC//////////////

void setCRC()
{
  CModBusCRC16 CRCengine;
  if (snd==1) setCRCa(CRCengine.process(24, cdhon_hash));
  setCRCb(CRCengine.process(34, cdhon_hash));
}

void checkCRC()
{
  CModBusCRC16 CRCengine;
  setCRCb(CRCengine.process(34, in_bytes));
}

void setCRCa(uint16_t CRC)
{
  cdhon_hash[24] = (CRC >> 8) & 0xff;   // MSB of CRC in Data[22]
  cdhon_hash[25] = (CRC >> 0) & 0xff;   // LSB of CRC in Data[23]
}

void setCRCb(uint16_t CRC)
{
  cdhon_hash[34] = (CRC >> 8) & 0xff;   // MSB of CRC in Data[22]
  cdhon_hash[35] = (CRC >> 0) & 0xff;   // LSB of CRC in Data[23]
}

