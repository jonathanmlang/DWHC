#include <main.h>

void initdraw(){

  oled.setFont(System5x7);
  oled.setCursor(2,0);
  oled.print("Water");
  oled.setCursor(50,0);
  oled.print("Fuel");
  oled.setCursor(93,0);
  oled.print("State");

  oled.setCursor(2,6);
  oled.print("Room");
  oled.setCursor(37,6);
  oled.print("Fuel Time");
 

  
}

void redraw(){

    drawbarfuel();
    drawfueltext();
    drawtemp();
    drawstate();
    drawfuelrate();
    drawroomtemp();
    drawfueltime();

}

int fuel_bar_low_ex=0;

void drawbarfuel(){

    byte barlength= 86;
    

    //float bar_low_level= tankmax/100;
    //      bar_low_level= bar_low_level*(float)bar_low_perc;
    
    float bar_multiplier= (float) tankmax / (float) barlength;

    fuel_bar= tankval / bar_multiplier;
    float low_bar = bar_low_level / bar_multiplier;
    int low_bar_int = low_bar;
    //int fuel_bar_low= (bar_low_level / bar_multiplier)+1;
    //fuel_bar_low_ex= fuel_bar_low;

    fuel_bar_int= fuel_bar;

    
    
    
    if ((menuselect==0 && fuel_bar_int!=fuel_bar_int_old) || (menuselect==0 && force)){
        oled.setFont(symbol);
        oled.setCursor(1,7);
        oled.print('O'); //fuel icon

       

        oled.setFont(bargraph);
        oled.setCursor(10,7);
        
        for (byte a=0; a<=barlength; a++){
            if (a==0 || a==barlength){oled.print('b');}
            else{
            if ((a< fuel_bar && (tankval>fuel_bar_low || blinker)) || a==0 || a==barlength) {
                if (a!=low_bar_int) {oled.print('a');}
                else      {oled.print('b');}
                }
            else{
                if (a!=low_bar_int) {oled.print('d');}
                else      {oled.print('e');}
                }
            }
        }
      fuel_bar_int_old= fuel_bar_int;
    }

    if (tankval_int <= bar_low_level){
      if (tankval!=0) {
        pinMode(A0,!blinker);
        digitalWrite(A0,!blinker);
        }
      else            {
        digitalWrite(A0,1);
        pinMode(A0,1);
        }
    }
    else{
      //digitalWrite(A0,0);
      //pinMode(A0,1);
    }
}    




void drawfueltext(){

  tankval_int=(int)(tankval);
  oled.setCursor(103,7);
  oled.setFont(numerals4x7);
  
  if (menupos==8){

      oled.setInvertMode(blinker || submenu);
      if (!(blinker || submenu)){
        oled.setFont(System5x7);
        oled.print(txt_max); oled.print("  ");
        oled.setFont(numerals4x7);
      }
      else{
        if (tankmax<1000){oled.print('@');} //space
        oled.print(tankmax /100);
        oled.print('.'); //.
        if (tankmax %100<10){oled.print(0);}
        oled.print(tankmax %100);
      }
      oled.setInvertMode(0);

  }

      oled.setInvertMode(menupos==1 && (blinker || submenu));
      if (tankval_int<1000){oled.print('@');} //space
      oled.print(tankval_int /100);
      oled.print('.'); //.
      if (tankval_int %100<10){oled.print(0);}
      oled.print(tankval_int %100);
      oled.setInvertMode(0);

}

void drawtemp(){

    if (menupos>1 && menupos<5){

       //min temp edit
          if (blinker || submenu){ 
            oled.setCursor(2,1);
            oled.setFont(lcdnums8x16);
            oled.setInvertMode(1);
            
            if (menupos==2) {if (tempset_h<10) {oled.print(' ');} oled.print(tempset_h);}
            if (menupos==3) {if (tempset_c<10) {oled.print(' ');} oled.print(tempset_c);}
            if (menupos==4) {if (tempset_t<10) {oled.print(' ');} oled.print(tempset_t);}
            oled.print(";=");
            oled.setInvertMode(0);
          }
          else{
            oled.setCursor(2,1);
            oled.setFont(tallletters);
            if (menupos==2) oled.print(txt_max);
            if (menupos==3) oled.print(txt_min);
            if (menupos==4) oled.print("SET");
          }

    }
    else{
      oled.setFont(lcdnums8x16);
      oled.setCursor(2,1);
      if (tempset_v<10) {oled.print(' ');}
      oled.print(tempset_v);
      oled.print(";=");
    }
    if (state==PREHEAT || state==RUN || state==COOL) {
      pinMode(7,1);
      pinMode(8,1);
      pinMode(A0,1);
    }
    else{
      pinMode(7,0);
      pinMode(8,0);
      pinMode(A0,0);
    }

    if (tempset_v < tempset_c){
      digitalWrite(7,1);
      digitalWrite(8,0);
      digitalWrite(A0,0);
    }
    else{
      if (tempset_v>tempset_t){
        digitalWrite(7,0);
        digitalWrite(8,0);
        digitalWrite(A0,1);
      }
      else{
        digitalWrite(7,0);
        digitalWrite(8,1);
        digitalWrite(A0,0);
      }
    }

}

void drawstate(){
  oled.setFont(tallletters);
  oled.setCursor(92,1);
  printstate(state);

  oled.setCursor(92,4);
  oled.setFont(symbol);
  oled.print(glow ? "34" : "  "); //glow icon 2 halves
  oled.setCursor(92,5);
  oled.print(badread ? "=" : " "); //show when a bad read occured
}

void printstate(byte val){

  switch(val){
    case RUN:       oled.print("RUN[");        break;
    case PREHEAT:   oled.print(blinker ? "PRE[" : "HEAT");        break;
    case IDLE:      oled.print("IDLE");        break;
    case COOL:      oled.print("COOL");        break;
    case ERROR:     oled.print("EROR");        break;
    case NONE:                                 break;
  }

}

void drawfuelrate(){

    if (menupos>4 && menupos<7){

       //min temp edit
          if (blinker || submenu){ 
            oled.setCursor(43,1);
            oled.setFont(lcdnums8x16);
            oled.setInvertMode(1);
            if (menupos==5) {
              
              oled.print(fuelrate_h/10);
              oled.print('.');
              oled.print(fuelrate_h%10);
            }
            if (menupos==6) {
              
              oled.print(fuelrate_l/10);
              oled.print('.');
              oled.print(fuelrate_l%10);
            }
            
          }
          else{
            oled.setCursor(43,1);
            oled.setFont(tallletters);
            if (menupos==5) {oled.print(txt_max); oled.print("[[");}
            if (menupos==6) {oled.print(txt_min); oled.print("[[");}
          }

          
          

    }
    else{

          oled.setFont(lcdnums8x16);
          oled.setCursor(43,1);
          if (fueluse<=0){
            oled.print("-.-");
          }
          else{
            oled.print(fueluse/10);
            oled.print('.');
            oled.print(fueluse%10);
          }
    }

    oled.setCursor(72,2);
    oled.setFont(System5x7);
    oled.setInvertMode(blinker && menupos==9);
    if (kwmode && !(menupos==5 || menupos==6)){
       oled.print("KW");
    }
    else{
       oled.print("Hz");
    }
    oled.setInvertMode(0);
}

void drawroomtemp(){

  oled.setFont(lcdnums8x16);
  oled.setCursor(2,4);

  roomtempcalc();

  if (menupos==7){
    oled.setInvertMode(blinker || submenu);
    oled.print(roomset);
    oled.setInvertMode(0);
  }
  else{   
    oled.print(roomtemp);
  }
  
  oled.print(";="); // "c "

}


void drawfueltime(){

  oled.setFont(lcdnums8x16);
  oled.setCursor(42,4);
  if (hours<=0 && mins<=0){
    oled.print("--:--");
  }
  else{
    if (hours<10) oled.print(0);
    oled.print(hours);
    oled.print(':');
    if (mins<10) oled.print(0);
    oled.print(mins);
  }
  //oled.print(total_mins);



  //draw diag
  //oled.setFont(System5x7);

  //oled.setCursor(92,3);
  //oled.print(analogRead(A6));
  //oled.setCursor(92,4);
  //oled.print(Setpoint);
  //oled.setCursor(92,5);
  //oled.print(cdhstat[8]);
  //oled.setCursor(92,6);
  //oled.print(Output);


  

}