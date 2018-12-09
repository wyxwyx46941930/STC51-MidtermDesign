#include "key.h"
#include "serial.h"

#define STATE_WAITING 0
#define STATE_VALIAD 1
#define STATE_INVALIAD 2
#define STATE_RESETTING 66

u8 curPos = 0;
u8 password[8] = {6,6,6,6,6,6,6,6};
u8 first_time = 1;
u8 info_true[8] = {26, 26, 26, 26, 26, 26, 26, 26};
u8 info_false[8] = {15, 15, 15, 15, 15, 15, 15, 15};
u8 i;
u8 reset_pos = 0;
u8 info_waitting[8] = {DIS_, DIS_DOT,DIS_,DIS_DOT,DIS_, DIS_DOT,DIS_};

u8 truePassword[8] = {1, 2, 3, 4, 5, 6, 7, 8};
u8 state = 0;


void DisplayInfo(u8* info) {
    u8 i;
    for(i = 0; i < 8; i++) {
        LED8[i] = info[i];
    }
}

u8 checkPassword() {
    u8 i;
    for(i = 0; i < 8; i++) {
        if(password[i] != truePassword[i]) {
            return 0;
        }
    }
    return 1;
}

void main(void) {
    //u8 send_buffer[2];
    u8 remoteKeyCode = 0;
    serial_init();
    // reset_led();
    key_init();
    msecond = 0;

    while(1)
    {
        if((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy))
        {
            remoteKeyCode = RX1_Buffer[TX1_Cnt];
            if(++TX1_Cnt >= UART1_BUF_LENGTH)   TX1_Cnt = 0;
        }

        if(B_1ms)   
        {
            B_1ms = 0;
            if(++msecond >= 200)
            {
                msecond = 0;
                switch(state) {
                    case STATE_WAITING:
                        DisplayInfo(info_waitting);
                        break;
                    case STATE_VALIAD:
                        DisplayInfo(info_true);
                        break;
                    case STATE_INVALIAD:
                        DisplayInfo(info_false);                        
                        break;
                    case STATE_RESETTING:
                        DisplayInfo(truePassword);
                    default: break;
                }
            }

            if(++cnt50ms >= 50)
            {
                cnt50ms = 0;
                IO_KeyScan();
            }

            if(curPos == 8) {
                curPos = 0;
                if(checkPassword()) {
                    state = STATE_VALIAD;
                    PrintU8('t');
                }
                else{
                    state = STATE_INVALIAD;
                    PrintU8('f');                                        
                }
            }

            if(remoteKeyCode != 255)
            {
                if(state != STATE_RESETTING && remoteKeyCode >= 0 && remoteKeyCode <= 9) {
                    if(first_time == 1) {
                        first_time = 0;
                    } else {
                        state = STATE_WAITING;
                        password[curPos] = remoteKeyCode;
                        curPos ++;
                    }
                } else if (remoteKeyCode == STATE_RESETTING) {
                    state = STATE_RESETTING;
                    //u8 i;
                    for(i = 0; i < 8;i++){
                        truePassword[i] = 17;
                    }
                } else if(state == STATE_RESETTING && remoteKeyCode > STATE_RESETTING && remoteKeyCode <= STATE_RESETTING + 10) {
                    truePassword[reset_pos++] = remoteKeyCode - 67;
                    if(reset_pos == 8) {
                        reset_pos = 0;
                        PrintU8('r');
                        state = STATE_WAITING;
                    }
                }
                remoteKeyCode = 255;
            }

            if(KeyCode != 0)
            {
                if (KeyCode == 27) {
                    state = STATE_RESETTING;
                    //u8 i;
                    for(i = 0; i < 8;i++){
                        truePassword[i] = 17;
                    }
                } else if(state == STATE_RESETTING && KeyCode >= 17 && KeyCode <= 17 + 9) {
                    truePassword[reset_pos++] = KeyCode - 17;
                    if(reset_pos == 8) {
                        reset_pos = 0;
                        state = STATE_WAITING;         
                    }
                }
                KeyCode = 0;
            }
        }
    }
}



