#ifndef STC_KEY_H_
#define STC_KEY_H_

// Using STC Library
/*************  ��������˵��  **************

��дRTC, ICΪPCF8563.

��STC��MCU��IO��ʽ����74HC595����8λ����ܡ�

�û������޸ĺ���ѡ��ʱ��Ƶ��.
ʹ��Timer0��16λ�Զ���װ������1ms����,�������������������, �û��޸�MCU��ʱ��Ƶ��ʱ,�Զ���ʱ��1ms.

�û���������ʾ�������޸ĳɹ�������.�Ƽ�����ʹ�ù��������.

8λ�������ʾʱ��(Сʱ-����-��).

����ɨ�谴������Ϊ17~32.

����ֻ֧�ֵ�������, ��֧�ֶ��ͬʱ����, ���������в���Ԥ֪�Ľ��.

�����³���1���,����10��/����ٶ��ṩ�ؼ����. �û�ֻ��Ҫ���KeyCode�Ƿ��0���жϼ��Ƿ���.

����ʱ���:
����17: Сʱ+.
����18: Сʱ-.
����19: ����+.
����20: ����-.


******************************************/

#include "common.h"

/***********************************************************/

#define DIS_DOT     0x20
#define DIS_BLACK   0x10
#define DIS_        0x11


/****************************** �û������ ***********************************/


#define Timer0_Reload   (65536UL -(MAIN_Fosc / 1000))       //Timer 0 �ж�Ƶ��, 1000��/��

/*****************************************************************************/

/*************  ���س�������    **************/
u8 code t_display[]={                       //��׼�ֿ�
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

u8 code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //λ��


/*************  IO�ڶ���    **************/
sbit    P_HC595_SER   = P4^0;   //pin 14    SER     data input
sbit    P_HC595_RCLK  = P5^4;   //pin 12    RCLk    store (latch) clock
sbit    P_HC595_SRCLK = P4^3;   //pin 11    SRCLK   Shift data clock


/*************  ���ر�������    **************/

u8  LED8[8];        //��ʾ����
u8  display_index;  //��ʾλ����
bit B_1ms;          //1ms��־

u8 IO_KeyState, IO_KeyState1, IO_KeyHoldCnt;    //���м��̱���
u8  KeyHoldCnt; //�����¼�ʱ
u8  KeyCode;    //���û�ʹ�õļ���, 1~16��Ч
u8  cnt50ms;

u8  hour,minute,second; //RTC����
u16 msecond;


/*************  ���غ�������    **************/
void    CalculateAdcKey(u16 adc);
void    IO_KeyScan(void);   //50ms call
void    WriteNbyte(u8 addr, u8 *p, u8 number);
void    ReadNbyte( u8 addr, u8 *p, u8 number);
void    DisplayRTC(void);
void    ReadRTC(void);
void    WriteRTC(void);

/********************** ��ʾʱ�Ӻ��� ************************/
void    DisplayRTC(void)
{
    if(hour >= 10)  LED8[0] = hour / 10;
    else            LED8[0] = DIS_BLACK;
    LED8[1] = hour % 10;
    LED8[2] = DIS_;
    LED8[3] = minute / 10;
    LED8[4] = minute % 10;
    LED8[6] = second / 10;
    LED8[7] = second % 10;
    LED8[5] = DIS_;
}

/********************** ��RTC���� ************************/
void    ReadRTC(void)
{
    u8  tmp[3];

    ReadNbyte(2, tmp, 3);
    second = ((tmp[0] >> 4) & 0x07) * 10 + (tmp[0] & 0x0f);
    minute = ((tmp[1] >> 4) & 0x07) * 10 + (tmp[1] & 0x0f);
    hour   = ((tmp[2] >> 4) & 0x03) * 10 + (tmp[2] & 0x0f);
}

/********************** дRTC���� ************************/
void    WriteRTC(void)
{
    u8  tmp[3];

    tmp[0] = ((second / 10) << 4) + (second % 10);
    tmp[1] = ((minute / 10) << 4) + (minute % 10);
    tmp[2] = ((hour / 10) << 4) + (hour % 10);
    WriteNbyte(2, tmp, 3);
}


/*****************************************************
    ���м�ɨ�����
    ʹ��XY����4x4���ķ�����ֻ�ܵ������ٶȿ�

   Y     P04      P05      P06      P07
          |        |        |        |
X         |        |        |        |
P00 ---- K00 ---- K01 ---- K02 ---- K03 ----
          |        |        |        |
P01 ---- K04 ---- K05 ---- K06 ---- K07 ----
          |        |        |        |
P02 ---- K08 ---- K09 ---- K10 ---- K11 ----
          |        |        |        |
P03 ---- K12 ---- K13 ---- K14 ---- K15 ----
          |        |        |        |
******************************************************/


u8 code T_KeyTable[16] = {0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0};

void IO_KeyDelay(void)
{
    u8 i;
    i = 60;
    while(--i)  ;
}

void    IO_KeyScan(void)    //50ms call
{
    u8  j;

    j = IO_KeyState1;   //������һ��״̬

    P0 = 0xf0;  //X�ͣ���Y
    IO_KeyDelay();
    IO_KeyState1 = P0 & 0xf0;

    P0 = 0x0f;  //Y�ͣ���X
    IO_KeyDelay();
    IO_KeyState1 |= (P0 & 0x0f);
    IO_KeyState1 ^= 0xff;   //ȡ��
    
    if(j == IO_KeyState1)   //�������ζ����
    {
        j = IO_KeyState;
        IO_KeyState = IO_KeyState1;
        if(IO_KeyState != 0)    //�м�����
        {
            F0 = 0;
            if(j == 0)  F0 = 1; //��һ�ΰ���
            else if(j == IO_KeyState)
            {
                if(++IO_KeyHoldCnt >= 20)   //1����ؼ�
                {
                    IO_KeyHoldCnt = 18;
                    F0 = 1;
                }
            }
            if(F0)
            {
                j = T_KeyTable[IO_KeyState >> 4];
                if((j != 0) && (T_KeyTable[IO_KeyState& 0x0f] != 0)) 
                    KeyCode = (j - 1) * 4 + T_KeyTable[IO_KeyState & 0x0f] + 16;    //������룬17~32
            }
        }
        else    IO_KeyHoldCnt = 0;
    }
    P0 = 0xff;
}




/**************** ��HC595����һ���ֽں��� ******************/
void Send_595(u8 dat)
{       
    u8  i;
    for(i=0; i<8; i++)
    {
        dat <<= 1;
        P_HC595_SER   = CY;
        P_HC595_SRCLK = 1;
        P_HC595_SRCLK = 0;
    }
}

/********************** ��ʾɨ�躯�� ************************/
void DisplayScan(void)
{   
    Send_595(~T_COM[display_index]);                //���λ��
    Send_595(t_display[LED8[display_index]]);   //�������

    P_HC595_RCLK = 1;
    P_HC595_RCLK = 0;                           //�����������
    if(++display_index >= 8)    display_index = 0;  //8λ������0
}


/********************** Timer0 1ms�жϺ��� ************************/
void timer0 (void) interrupt 1
{
    DisplayScan();  //1msɨ����ʾһλ
    B_1ms = 1;      //1ms��־
}



#define SLAW    0xA2
#define SLAR    0xA3

sbit    SDA = P1^1; //����SDA  PIN5
sbit    SCL = P1^0; //����SCL  PIN6

/****************************/
void    I2C_Delay(void) //for normal MCS51, delay (2 * dly + 4) T, for STC12Cxxxx delay (4 * dly + 10) T
{
    u8  dly;
    dly = MAIN_Fosc / 2000000UL;        //��2us����
    while(--dly)    ;
}

/****************************/
void I2C_Start(void)               //start the I2C, SDA High-to-low when SCL is high
{
    SDA = 1;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SDA = 0;
    I2C_Delay();
    SCL = 0;
    I2C_Delay();
}       


void I2C_Stop(void)                 //STOP the I2C, SDA Low-to-high when SCL is high
{
    SDA = 0;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SDA = 1;
    I2C_Delay();
}

void S_ACK(void)              //Send ACK (LOW)
{
    SDA = 0;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SCL = 0;
    I2C_Delay();
}

void S_NoACK(void)           //Send No ACK (High)
{
    SDA = 1;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SCL = 0;
    I2C_Delay();
}
        
void I2C_Check_ACK(void)         //Check ACK, If F0=0, then right, if F0=1, then error
{
    SDA = 1;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    F0  = SDA;
    SCL = 0;
    I2C_Delay();
}

/****************************/
void I2C_WriteAbyte(u8 dat)     //write a byte to I2C
{
    u8 i;
    i = 8;
    do
    {
        if(dat & 0x80)  SDA = 1;
        else            SDA = 0;
        dat <<= 1;
        I2C_Delay();
        SCL = 1;
        I2C_Delay();
        SCL = 0;
        I2C_Delay();
    }
    while(--i);
}

/****************************/
u8 I2C_ReadAbyte(void)          //read A byte from I2C
{
    u8 i,dat;
    i = 8;
    SDA = 1;
    do
    {
        SCL = 1;
        I2C_Delay();
        dat <<= 1;
        if(SDA)     dat++;
        SCL  = 0;
        I2C_Delay();
    }
    while(--i);
    return(dat);
}

/****************************/
void WriteNbyte(u8 addr, u8 *p, u8 number)          /*  WordAddress,First Data Address,Byte lenth   */
                                                            //F0=0,right, F0=1,error
{
    I2C_Start();
    I2C_WriteAbyte(SLAW);
    I2C_Check_ACK();
    if(!F0)
    {
        I2C_WriteAbyte(addr);
        I2C_Check_ACK();
        if(!F0)
        {
            do
            {
                I2C_WriteAbyte(*p);     p++;
                I2C_Check_ACK();
                if(F0)  break;
            }
            while(--number);
        }
    }
    I2C_Stop();
}


/****************************/
void ReadNbyte(u8 addr, u8 *p, u8 number)       /*  WordAddress,First Data Address,Byte lenth   */
{
    I2C_Start();
    I2C_WriteAbyte(SLAW);
    I2C_Check_ACK();
    if(!F0)
    {
        I2C_WriteAbyte(addr);
        I2C_Check_ACK();
        if(!F0)
        {
            I2C_Start();
            I2C_WriteAbyte(SLAR);
            I2C_Check_ACK();
            if(!F0)
            {
                do
                {
                    *p = I2C_ReadAbyte();   p++;
                    if(number != 1)     S_ACK();    //send ACK
                }
                while(--number);
                S_NoACK();          //send no ACK
            }
        }
    }
    I2C_Stop();
}
/****************************/

void key_init(void) {
    u8 i;
    P0M1 = 0;   P0M0 = 0;   //����Ϊ׼˫���
    P1M1 = 0;   P1M0 = 0;   //����Ϊ׼˫���
    P2M1 = 0;   P2M0 = 0;   //����Ϊ׼˫���
    P3M1 = 0;   P3M0 = 0;   //����Ϊ׼˫���
    P4M1 = 0;   P4M0 = 0;   //����Ϊ׼˫���
    P5M1 = 0;   P5M0 = 0;   //����Ϊ׼˫���
    P6M1 = 0;   P6M0 = 0;   //����Ϊ׼˫���
    P7M1 = 0;   P7M0 = 0;   //����Ϊ׼˫���
    
    display_index = 0;
    
    AUXR = 0x80;    //Timer0 set as 1T, 16 bits timer auto-reload, 
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    ET0 = 1;    //Timer0 interrupt enable
    TR0 = 1;    //Tiner0 run
    EA = 1;     //�����ж�
    
    for(i=0; i<8; i++)  LED8[i] = 0x10; //�ϵ�����

    
    ReadRTC();
    F0 = 1;
    if(second >= 60)    F0 = 1; //����
    if(minute >= 60)    F0 = 1; //����
    if(hour   >= 24)    F0 = 1; //����
    if(F0)  //�д���, Ĭ��12:00:00
    {
        second = 39;
        minute = 39;
        hour  = 12;
        WriteRTC();
    }

    DisplayRTC();
    LED8[2] = DIS_;
    LED8[5] = DIS_;

    KeyHoldCnt = 0; //�����¼�ʱ
    KeyCode = 0;    //���û�ʹ�õļ���, 1~16��Ч

    IO_KeyState = 0;
    IO_KeyState1 = 0;
    IO_KeyHoldCnt = 0;
    cnt50ms = 0;
}

#endif  // STC_KEY_H_
