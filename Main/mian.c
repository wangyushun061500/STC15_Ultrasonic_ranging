#include "config.h"
#include "INTRINS.H"

// 提前声明含糊

void Delay20us();       //20us延时函数，24MHz
void Delay1000ms();     //1s延时函数，24MHz

void main (void)
{
    P5M0 = 0x30;
    P5M1 = 0x00;
    P1M0 = 0x60;
    P1M1 = 0x00;
    Delay20us();
    LED_OUT = 1;
    SR_04_Vcc = 1;
    SR_04_Gnd = 0;
    while (1)
    {
        SR_04_Trig = 1;
        Delay20us();
        SR_04_Trig = 0;
        while(SR_04_Echo)
        {
            LED_OUT = 0;
        }
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
