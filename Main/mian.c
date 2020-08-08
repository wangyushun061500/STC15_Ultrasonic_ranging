#include "config.h"
#include "INTRINS.H"


// �������ڴ�������

#define FOSC 24000000L		//ϵͳƵ��
#define BAUD 115200 		//������
#define NONE_PARITY     0       //��У��
#define ODD_PARITY      1       //��У��
#define EVEN_PARITY     2       //żУ��
#define MARK_PARITY     3       //���У��
#define SPACE_PARITY    4       //�հ�У��
#define PARITYBIT NONE_PARITY   //����У��λ
#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7


// ��������

bit busy;
unsigned int temp;
float Fl;

// ��ǰ��������

void Delay20us();       //20us��ʱ������24MHz
void Delay40ms();
void Delay1000ms();     //1s��ʱ������24MHz
void Timer0Init(void);
void uart_init();
void SendData(unsigned char dat);
void SendString(char *s);
void serial_one_send_number(long num);
void serial_one_send_float(double float_val, char bit_val);

void main (void)
{
    unsigned int count;
    P5M0 = 0x30;
    P5M1 = 0x00;
    P1M0 = 0x60;
    P1M1 = 0x00;
    Delay20us();
    Timer0Init();
    uart_init();
	T2L = (65536 - (FOSC/4/BAUD));   //���ò�������װֵ
    T2H = (65536 - (FOSC/4/BAUD))>>8;
    AUXR = 0x14;                //T2Ϊ1Tģʽ, ��������ʱ��2
    AUXR |= 0x01;               //ѡ��ʱ��2Ϊ����1�Ĳ����ʷ�����
    ES = 1;                     //ʹ�ܴ���1�ж�
    ET0 = 1;
    EA = 1;
	Delay1000ms();
    SendString("STC15F2K60S2\r\nUart ON !\r\n");
    BZ_OUT = 0;
    SR_04_Vcc = 1;
    SR_04_Gnd = 0;
    temp = 0;
    while (1)
    {
        Fl = (float)(temp - count);
        Fl = Fl*3.4/20;
        count = temp;
        SR_04_Trig = 0;
        Delay20us();
        SR_04_Trig = 1;;
		if (Fl >= 400)
		{
			Fl = 0;
		}
		
        serial_one_send_float(Fl,1);
        SendString("cm\r\n");
        if (Fl < 170)
        {
            BZ_OUT = 1;
        }else
        {
            BZ_OUT = 0;
        }
        Delay40ms();
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

void Delay40ms()		//@24.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 4;
	j = 166;
	k = 210;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
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

/* t0��ʼ������24MHz��ʱ1us */
void Timer0Init(void)		//1΢��@24.000MHz
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0xE8;		//���ö�ʱ��ֵ
	TH0 = 0xFF;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
}

/*uart��ʼ������*/
void uart_init ()
{
	ACC = P_SW1;
    ACC &= ~(S1_S0 | S1_S1);    //S1_S0=0 S1_S1=0
    P_SW1 = ACC;                //(P3.0/RxD, P3.1/TxD)
	#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;                //8λ�ɱ䲨����
	#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xda;                //9λ�ɱ䲨����,У��λ��ʼΪ1
	#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xd2;                //9λ�ɱ䲨����,У��λ��ʼΪ0
	#endif
}

/*�������ݷ���*/
void SendData(unsigned char dat)
{
    while (busy);               //�ȴ�ǰ������ݷ������
    ACC = dat;                  //��ȡУ��λP (PSW.0)
    if (P)                      //����P������У��λ
    {
		#if (PARITYBIT == ODD_PARITY)
			TB8 = 0;                //����У��λΪ0
		#elif (PARITYBIT == EVEN_PARITY)
			TB8 = 1;                //����У��λΪ1
		#endif
    }
    else
    {
		#if (PARITYBIT == ODD_PARITY)
    	    TB8 = 1;                //����У��λΪ1
		#elif (PARITYBIT == EVEN_PARITY)
    	    TB8 = 0;                //����У��λΪ0
		#endif
    }
    busy = 1;
    SBUF = ACC;                 //д���ݵ�UART���ݼĴ���
}

/*�����ַ���*/
void SendString(char *s)
{
    while (*s)                  //����ַ���������־
    {
        SendData(*s++);         //���͵�ǰ�ַ�
    }
}

/*uart����long��ֵ*/
void serial_one_send_number(long num)
{
	long dat = 0;
	unsigned char  length = 0;
	if(num < 0)										//����ֵΪ����ʱ
	{
		SendData('-');	//�������
		num = -num;									//����ֵȡ�෴��
	}
	
	if(num == 0)									//����ֵΪ0ʱ
		SendData('0');	//����ַ�0
	else											//����ֵ��Ϊ0ʱ
	{
		while(num)									//����ֵ������
		{
			dat = dat * 10;
			dat = dat + num % 10;
			num = num / 10;
			length++;
		}
		
		while(length--)							//�ӵ�һλ��ʼ�������������ֵ
		{
			SendData(dat % 10 + '0');
			dat = dat / 10;
		}
	}
}


void serial_one_send_float(double float_val, char bit_val)
{
	long xdata value_int = 0;
	long xdata value_flt = 0;
	
	if(float_val < 0)
	{
		SendData('-');
		float_val = -float_val;
	}
	
	value_int = (long)float_val;
	
	float_val = float_val - (double)value_int;
	
	for(;bit_val;bit_val--)
		float_val = float_val * 10;
	
	serial_one_send_number(value_int);
	SendData('.');
	serial_one_send_number((long)float_val);
}


/* t0�жϷ������ */
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

/*uart�жϷ������*/
void Uart() interrupt 4
{
    if (RI)
    {
        RI = 0;                 //���RIλ
        P0 = SBUF;              //P0��ʾ��������
        P54 = RB8;              //P2.2��ʾУ��λ
    }
    if (TI)
    {
        TI = 0;                 //���TIλ
        busy = 0;               //��æ��־
    }
}