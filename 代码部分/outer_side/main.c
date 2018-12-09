#include "key.h"
#include "serial.h"

#define STATE_INPUTTING 0
#define STATE_VALIAD 1
#define STATE_INVALIAD 2

u8 curPos = 0;
u8 first_time = 1;
u8 info_true[8] = {26, 26, 26, 26, 26, 26, 26, 26};
u8 info_false[8] = {15, 15, 15, 15, 15, 15, 15, 15};
u8 state = 0;
u8 password[8];

void reset_led(){
    u8 i;
    for(i=0;i<8;i++){
        LED8[i] = DIS_;
    }
}

void DisplayInfo(u8* info) {
    u8 i;
    for(i = 0; i < 8; i++) {
        LED8[i] = info[i];
    }
}

void initPassword(){
    u8 i;
    curPos = 0;
    for(i = 0; i < 8; i++) {
        password[i] = DIS_;
    }
    password[8] = 0;
}

void main(void) {

//    u8 send_buffer[2];
    u8 remoteKeyCode = 0;
    serial_init();
    key_init();
	initPassword();

    while(1)
    {
        // 检测是否收到数据
        if((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy))
        {
            // 收到数据，并将数据中的键码赋给变量remoteKeyCode
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
                    case STATE_INPUTTING:
                        DisplayInfo(password);
                        break;
                    case STATE_VALIAD:
                        DisplayInfo(info_true);
                        break;
                    case STATE_INVALIAD:
                        DisplayInfo(info_false);                
                        break;
                    default: break;
                }
            }

            if(++cnt50ms >= 50)
            {
                cnt50ms = 0;
                IO_KeyScan();
            }

            if(KeyCode != 0)
            {
                if(state == STATE_INPUTTING && KeyCode >= 17 &&  KeyCode <= 26)              
                {
                    password[curPos++] = KeyCode % 17;
                } else if(KeyCode == 27) {
                    if(curPos == 8) {
                       	PrintString1(password);
                    }
                } else if(KeyCode == 28) {
                    state = STATE_INPUTTING;
                    initPassword();
                }
                KeyCode = 0;
            }

            if(remoteKeyCode != 0) {

                if(remoteKeyCode == 't') {
                    state = STATE_VALIAD;
                } else if (remoteKeyCode == 'f') {
                    state = STATE_INVALIAD;                    
                }

                remoteKeyCode = 0;
            }
        }
    }
}
