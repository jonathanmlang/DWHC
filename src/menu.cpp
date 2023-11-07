#include <main.h>

bool wassubmenu=0;

void menufn(){

    btnUP.read();
    btnDN.read();
    btnSEL.read();

    bool up_     =btnUP.wasPressed();
    bool up_rpt  =btnUP.pressedFor(500);
    bool sel_    =btnSEL.wasPressed();
    bool sel_rpt =btnSEL.pressedFor(1000);
    //bool sel_long=btnSEL.pressedFor(5000);
    bool dn_     =btnDN.wasPressed();
    bool dn_rpt  =btnDN.pressedFor(500);

    bool roomset_ch=0; //check that room temp setpoint was changed

    if (menuselect==0 && sel_){
        if (submenu && menupos>0){
            writeEeprom(0);
        }
        submenu=!submenu;
    }

    if (submenu){
      if (menuselect==0){  
        wassubmenu=1;
        if (menupos==0){
            if (sel_){
                if (state==IDLE && !fuelout){
                    send=ON;
                    cmd_send(send);
                }
                else{
                    send=OFF;
                    cmd_send(send);
                }
                submenu=0;
            }
        }
        if (menupos==1){
            if (up_ || up_rpt){
                if (up_rpt) {tankval= min(tankval+10, tankmax);}
                else        {tankval= min(tankval+1,  tankmax);}
            }
            if (dn_ || dn_rpt){
                if (dn_rpt) {tankval= max(tankval-10, 0);}
                else        {tankval= max(tankval-1,  0);}
            }
        }

        if (menupos==2){ //tempset hot
            if (up_ || up_rpt){
                tempset_h= min(tempset_h+1,  90);
            }
            if (dn_ || dn_rpt){
                tempset_h= max(tempset_h-1,  tempset_t);
            }
        }

        if (menupos==3){ //tempset set
            if (up_ || up_rpt){
                tempset_c= min(tempset_c+1,  tempset_t);
            }
            if (dn_ || dn_rpt){
                tempset_c= max(tempset_c-1,  0);
            }
        }

        if (menupos==4){ //tempset low
            if (up_ || up_rpt){
                tempset_t= min(tempset_t+1,  tempset_h);
            }
            if (dn_ || dn_rpt){
                tempset_t= max(tempset_t-1,  tempset_c);
            }
        }


        if (menupos==5){ //fuel max hz
            if (up_ || up_rpt){
                fuelrate_h= min(fuelrate_h+1,  99);
            }
            if (dn_ || dn_rpt){
                fuelrate_h= max(fuelrate_h-1,  fuelrate_l);
            }
        }

        if (menupos==6){ //fuel min hz
            if (up_ || up_rpt){
                fuelrate_l= min(fuelrate_l+1,  fuelrate_h);
            }
            if (dn_ || dn_rpt){
                fuelrate_l= max(fuelrate_l-1,  10);
            }
        }

        if (menupos==7){ //room temp
            if (up_ || up_rpt){
                roomset= min(roomset+1,  32);
                roomset_ch=1;
            }
            if (dn_ || dn_rpt){
                roomset= max(roomset-1,  0);
                roomset_ch=1;
            }
        }

        if (menupos==8){ //tank max level
            if (up_ || up_rpt){
                if (up_rpt) {tankmax= min(tankmax+10, 9999);}
                else        {tankmax= min(tankmax+1,  9999);}
            }
            if (dn_ || dn_rpt){
                if (dn_rpt) {tankmax= max(tankmax-10, 0);}
                else        {tankmax= max(tankmax-1,  0);}
            }
        }

        if (menupos==9){
            if (sel_){
                kwmode=!kwmode;
                submenu=0;
            }
        }

        

      }
    }
    else{
        if (up_){
            menupos=min(menupos+1, numopt);
        }

        if (dn_){
            menupos=max(menupos-1,0);
        }

        if (wassubmenu){
            //write vals to serial buffer
            setvals();
            wassubmenu=0;
        }
    }

    if (roomset_ch){
        Setpoint= (roomset + 50) / 0.1059;
    }


    if (
        up_     ||
        up_rpt  ||
        sel_    ||
        sel_rpt ||
        dn_     ||
        dn_rpt   
    )
    {
        redraw();
    }

    
}