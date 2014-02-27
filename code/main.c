#include "main.h"

time_t *time = &gtime;

uint8_t timeshow(uint8_t ltime) {
    uint8_t show = 0;
    for(uint8_t i = 0; i <= 6; i++) {
        if (ltime & (1 << (5 - i))) {
            show |= (1 << i);
        }
    }
    return show;
}
void Zrpos() {
    if (time->s == 60) {
        time->s = 0;
        time->m++;
    }
    if (time->m == 60) {
        time->m = 0;
        time->h++;
    }
    if (time->h == 24) {
        time->h = 0;
    }
}

void Zrneg() {
    if (time->s == 255) {
        time->s = 59;
        time->m--;
    }
    if (time->m == 255) {
        time->m = 59;
        if (time->h > 0) {
            time->h--;
        }
    }
    if (time->h == 255) {
        time->h = 23;
        time->m = 59;
    }
}

void init(void) {
    //AUS/Eingänge festlegen
    TIMELEDDDR |= TIMELEDS;
    TIMESLEDDDR |= SLED;
    TIMELEDGNDDDR |= TIMELED1GND | TIMELED2GND;
    TASTERDDR &= ~(TASTER | DREH1 | DREH2);
    TIMELEDGNDP |= TIMELED1GND | TIMELED2GND;
    ALARMDDR|= ALARM1 | ALARM2;
    TASTERP |= DREH1 | DREH2 | TASTER;  //Taster auf aktive Low setzen

    OCR1AH = 244;       //Timer1_ Comp
    OCR1AL = 35;            //Timer1_ Comp  auf 62499 gestellt->Interrupt bei 62500; 8000000/62500/sekszori(128) ergibt 1s
    TCCR1B |= (1<<WGM12) | (1<<CS10);       //Timer1 auf CTC setzen und Timer1 prescaler setzten
    TCCR0B |= (1<<CS00);            //precaler für Timer0 auf /1 setzten
    TIMSK |= (1<<OCIE1A) | (1<<TOIE0);      //  Timer1 Init(Compare Match mode) und Timer0 Overflow Interrupt
    GIMSK |= (1<<PCIE);     //Global Interrupt Flag für PCINT setzen
    //PCMSK |= (1<<PCINT0) | (1<<PCINT1) | (1<<PCINT2);
    time->s = 0;        //Startzeit nach Init festlegen
    time->m = 5;
    time->h = 0;
    sekz = SEKZORI; //= (uint8_t)((1000000/1)/(OCR1H*256+OCR1L));
}

void statechange(uint8_t state) {
    if (state == AUS) {
        status = AUS;
        //leds off?
    }
    else if (state == CHO) {
        if (status == ALARM) {
            time->s = 0;
            time->m = 5;
            time->h = 0;
            sekz = SEKZORI;
            ALARMP &= ~(ALARM1|ALARM2);
        }
        status = CHO;
        TIMESLEDPORT &= ~SLED;
    }
    else if (state == RUN) {
        status = RUN;
    }
    else if (state == ALARM) {
        status = ALARM;
        sekz = SEKZORI/2;
        ALARMP |= ALARM1;
        ALARMP &= ~ALARM2;
        TIMESLEDPORT &= ~SLED;
    }
}

ISR (TIMER1_COMPA_vect) {
    comp();
}

void comp() {
    /*if (buttonstate == NLL) {
        count3 = (++count3) % (6);
        if (count3 == 0) {
            buttonstate = NONE;
            if ((PINB & TASTERP) == 0x00) {
                switch(status) {
                    case CHO: statechange(RUN); break;
                    case RUN: statechange(CHO); break;
                    case ALARM: statechange(CHO); break;
                }
            }
        }
    }*/
    count2 = (++count2) % sekz;
    if (status == ALARM && (count2 == ALARMDUTY)) {
        alarmstate = 1;
    }
    else if (count2 == 0) {
        alarmstate = 0;
    }

    if (status == RUN && count2 == 0) {
        time->s--;
        if (time->s == 0 && time->m == 0 && time->h == 0) {
            statechange(ALARM);
        }
        else {
            Zrneg();
        }
        if (time->h>0) {
            TIMESLEDPORT |=SLED;
        }
        else{
            TIMESLEDPORT &=~SLED;
        }
    }
    if (status==RUN && (count2==(sekz/2))) {
        TIMESLEDPORT &=~SLED;
    }

}

void buttoncheck(void) {
    uint8_t buttons = PINB & 0x07;
    if (button_state != BUT_LONG && button_state != BUT_SHORT) {
        if ((buttons & TASTER) != 0x04) {
            if (button_state != BUT_HOLD) {
                button_state = BUT_PRESSED;
                button_count++;
                if (button_count >= 800) {
                    button_state = BUT_LONG;
                }
            }
        }
        else{
            if (button_state == BUT_PRESSED && button_count >= 100) {
                button_state = BUT_SHORT;
            }
            else{
                button_state = BUT_NONE;
            }
        }
    }
    uint8_t zust = (buttons & 0x03);
    if (zust != zust_state) {
        if (zust == zust_new_state) {
            zust_count++;
        }
        else{
            zust_count = 0;
            zust_new_state = zust;
        }
        if (zust_count == 5) {
            zust_state=zust_new_state;

            if ((zust_state == DREH2) && dstate != 22) {
                dstate = 11;
            }
            else if ( zust_state == DREH1 && dstate != 12) {
                dstate = 21;
            }
            else if (dstate == 12 && zust_state == DREH1) {
                dstate = 13;
            }
            else if (dstate == 21 && zust_state == 0) {
                dstate = 22;
            }
            else if (dstate == 23 && zust_state == DREH12) {
                dstate = 24;
            }
            else if (dstate == 11 && zust_state == 0) {
                dstate = 12;
            }
            else if (dstate == 13 && zust_state == DREH12) {
                dstate = 14;
            }

            else if (dstate == 22 && zust_state == DREH2) {
                dstate = 23;
            }
            else{
                dstate=0;
            }
        }
    }
}


ISR(TIMER0_OVF_vect) {
    count++;
    if (count % 8 == 0) {
        buttoncheck();
    }
    if (count == 0)
        counterflag = NLL;
    if (count == SPWM_V)
        counterflag = L2;
    else if (count == 2 * SPWM_V - 1)
        counterflag = RES;
    if (count % ALARMFREQUENCY == 0 && alarmstate == 1) {
        ALARMP ^= ALARM1;
        ALARMP ^= ALARM2;
    }
}

int main(void) {
    init();
    statechange(CHO);
    sei();
    while(1)
    {

        if (button_state == BUT_LONG || button_state == BUT_SHORT) {
            button_state = BUT_HOLD;//or NONE so
            button_count = 0;
            switch(status) {
                case CHO: statechange(RUN); break;
                case RUN: statechange(CHO); break;
                case ALARM: statechange(CHO); break;
            }
        }
        /*else if (button_state == BUT_SHORT) {
            button_state = BUT_NONE;
            button_count = 0;
        }*/
        if (status == AUS) {
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);
            sleep_mode();
            set_sleep_mode(SLEEP_MODE_IDLE);
        }
        else if (status == CHO) {
            if (dstate == 24) {
                dstate = 0;
                time->m++;
                time->s = 0;
                Zrpos();
            }
            else if (dstate == 14) {
                dstate = 0;
                if (time->m > 1 || time->h > 0) {
                    time->m--;
                    Zrneg();
                }
                time->s = 0;
            }
        }
        if (status == RUN || status == CHO) {
            if (counterflag == NLL) {
                counterflag = NONE;
                TIMELEDPORT = 0x00;
                TIMELEDGNDP &= ~TIMELED2GND;    //GND1 deaktivieren
                TIMELEDGNDP |= TIMELED1GND;     //GND2 aktivieren
                if (time->h > 0) {
                    TIMELEDPORT = timeshow(time->h);    //unten //Daten an PORT anlegen
                }
                else{
                    TIMELEDPORT = timeshow(time->m);
                }
            }
            else if (counterflag == L2) {
                counterflag = NONE;
                TIMELEDPORT = 0x00;
                TIMELEDGNDP |= TIMELED2GND;     //GND1 aktivieren
                TIMELEDGNDP &= ~TIMELED1GND;    //GND2 deaktivieren
                if (time->h > 0) {
                    TIMELEDPORT = timeshow(time->m);    //unten //Daten an PORT anlegen
                }
                else{
                    TIMELEDPORT = timeshow(time->s);
                }
            }
            else if (counterflag == RES) {
                counterflag = NONE;
                TIMELEDPORT &= ~TIMELEDS;
                TIMELEDGNDP &= ~(TIMELED1GND|TIMELED2GND);
            }
        }
        else if (status == ALARM) {
            if (alarmstate == 1) {
                if (counterflag == NLL || counterflag == L2) {
                    counterflag = NONE;
                    TIMELEDPORT |= TIMELEDS;
                    TIMELEDGNDP |= TIMELED1GND | TIMELED2GND;
                }
                else if (counterflag == RES) {
                    counterflag = NONE;
                    TIMELEDPORT &= ~TIMELEDS;
                    TIMELEDGNDP &= ~(TIMELED1GND | TIMELED2GND);
                }
            }
            else if (alarmstate == 0) {
                TIMELEDGNDP &= ~(TIMELED1GND | TIMELED2GND);
                TIMELEDPORT &= ~TIMELEDS;
            }
        }
        sleep_mode();
    }
    return 0;
}
