#include "config.h"
#include "INTRINS.H"


// 声明串口传输设置

#define FOSC 24000000L		//系统频率
#define BAUD 115200 		//波特率
#define NONE_PARITY     0       //无校验
#define ODD_PARITY      1       //奇校验
#define EVEN_PARITY     2       //偶校验
#define MARK_PARITY     3       //标记校验
#define SPACE_PARITY    4       //空白校验
#define PARITYBIT NONE_PARITY   //定义校验位
#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7


// 变量声明

bit busy;
unsigned int temp;


// 提前声明含糊

void Delay20us();       //20us延时函数，24MHz
void Delay1000ms();     //1s延时函数，24MHz
void Timer0Init(void);
void uart_init();
void SendData(unsigned char dat);
void SendString(char *s);

void main (void)
{
    unsigned char i;
    unsigned int count,Difference;
    P5M0 = 0x30;
    P5M1 = 0x00;
    P1M0 = 0x60;
    P1M1 = 0x00;
    Delay20us();
    Timer0Init();
    uart_init();
	T2L = (65536 - (FOSC/4/BAUD));   //设置波特率重装值
    T2H = (65536 - (FOSC/4/BAUD))>>8;
    AUXR = 0x14;                //T2为1T模式, 并启动定时器2
    AUXR |= 0x01;               //选择定时器2为串口1的波特率发生器
    ES = 1;                     //使能串口1中断
    ET0 = 1;
    EA = 1;
	Delay1000ms();
    SendString("STC15F2K60S2\r\nUart ON !\r\n");
    LED_OUT = 1;
    SR_04_Vcc = 1;
    SR_04_Gnd = 0;
    temp = 0;
    while (1)
    {
        i++;
        if (i == 10)
        {
            i = 0;
        }
        Difference = (temp - count);
        count = temp;
        SR_04_Trig = 1;
        Delay20us();
        SR_04_Trig = 0;
        SendString("第");
        SendData('0' + i);
        SendString("次循环\r\n");
        SendData('0' + Difference/10000);
        SendData('0' + Difference/1000/10);
        SendData('0' + Difference/1000%10);
        SendData('0' + Difference/100%10);
        SendData('0' + Difference%10);
        SendString("\r\n");
        Delay1000ms();

    }
    

}

void Delay20us()		//@24.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 117;
	while (--i);
}

void Delay1000ms()		//@24.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 92;
	j = 50;
	k = 238;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/* t0初始化程序，24MHz定时1us */
void Timer0Init(void)		//1微秒@24.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xE8;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

/*uart初始化函数*/
void uart_init ()
{
	ACC = P_SW1;
    ACC &= ~(S1_S0 | S1_S1);    //S1_S0=0 S1_S1=0
    P_SW1 = ACC;                //(P3.0/RxD, P3.1/TxD)
	#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;                //8位可变波特率
	#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xda;                //9位可变波特率,校验位初始为1
	#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xd2;                //9位可变波特率,校验位初始为0
	#endif
}

/*串口数据发送*/
void SendData(unsigned char dat)
{
    while (busy);               //等待前面的数据发送完成
    ACC = dat;                  //获取校验位P (PSW.0)
    if (P)                      //根据P来设置校验位
    {
		#if (PARITYBIT == ODD_PARITY)
			TB8 = 0;                //设置校验位为0
		#elif (PARITYBIT == EVEN_PARITY)
			TB8 = 1;                //设置校验位为1
		#endif
    }
    else
    {
		#if (PARITYBIT == ODD_PARITY)
    	    TB8 = 1;                //设置校验位为1
		#elif (PARITYBIT == EVEN_PARITY)
    	    TB8 = 0;                //设置校验位为0
		#endif
    }
    busy = 1;
    SBUF = ACC;                 //写数据到UART数据寄存器
}

/*发送字符串*/
void SendString(char *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}

/* t0中断服务程序 */
void T0_isr() interrupt 1
{
    if (SR_04_Echo == 1)
    {
        temp++;
        if(temp == 65535)
        {
            temp = 0;
        }
    }
    
}

/*uart中断服务程序*/
void Uart() interrupt 4
{
    if (RI)
    {
        RI = 0;                 //清除RI位
        P0 = SBUF;              //P0显示串口数据
        P54 = RB8;              //P2.2显示校验位
    }
    if (TI)
    {
        TI = 0;                 //清除TI位
        busy = 0;               //清忙标志
    }
}