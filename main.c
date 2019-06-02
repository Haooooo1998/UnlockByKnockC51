#include <reg52.h>
#include <math.h>
//#include <STC12C5A60S2.h>
//#define uchar unsigned char
//#define uint unsigned int

sbit redLED = P1 ^ 6;
sbit yellowLED = P1 ^ 3;
sbit blueLED = P1 ^ 0;
sbit key = P0 ^ 6;
sbit pwm = P3 ^ 0;
sbit programbutton = P3 ^ 5;

int maxknock = 10;																					//最大敲击数
int rightknocktime[10] = {100, 45, 0, 0, 0, 0, 0, 0, 0, 0}; //正确的数组
int tempknocktime[10] = {0};																//记录敲击时间的数组
int knocknumber = 0;
int num = 0; //时间1ms
int endflag = 0;
int firstknock = 0; //是不是第一次检测到敲击
int motornum = 0;
int motorstop = 0;
int motorturnflag = 0;

// 映射函数
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//延时函数大约1ms
void delay(unsigned int z)
{
	unsigned int i, j;
	for (i = z; i > 0; i--)
		for (j = 110; j > 0; j--)
			;
}

//初始化函数
void initial_main()
{
	blueLED = 0;
	yellowLED = 0;
	redLED = 0;
	EA = 1;
	ET0 = 1;		 //允许定时器T0中断
	ET1 = 1;		 //允许定时器T1中断
	TMOD = 0x11; //开启定时器T0，T1，方式1
	TH0 = 0xfc;	//装载初值，1ms,晶振12M
	TL0 = 0x18;
	TH1 = 0xfe; //装载初值，0.5ms,晶振12M
	TL1 = 0x0c;
}

//记录敲击时间的函数
void listenknock()
{
	num = 0;
	TR0 = 1;
	knocknumber++;
	endflag = 0;
	blueLED = 1;
	delay(100);
	blueLED = 0;
	while (1)
	{
		if (num > 4000) //超时结束
		{
			TR0 = 0;
			endflag = 1; //结束标志
			break;
		}
		if (key == 0) //下一次震动
		{
			TR0 = 0;
			break;
		}
		if (knocknumber > 10) //超过最大敲击数结束
		{
			TR0 = 0;
			endflag = 1;
			break;
		}
	}
	if (endflag == 0)
	{
		tempknocktime[knocknumber - 1] = num;
	}
}

//判断函数
int checkknocktime()
{
	int i = 0;
	int knockcount = 0;
	int tempknockcount = 0;
	int maxknocktime = 0;
	int totaltimediff = 0;
	int timediff = 0;

	endflag = 0; //重置
	knocknumber = 0;
	firstknock = 0;

	for (i = 0; i < maxknock; i++)
	{
		if (tempknocktime[i] > 0)
		{
			tempknockcount++;
		}
		if (rightknocktime[i] > 0)
		{
			knockcount++;
		}
		if (tempknocktime[i] > maxknocktime)
		{
			maxknocktime = tempknocktime[i];
		}
	}
	if (knockcount != tempknockcount)
	{
		return 0;
	}
	for (i = 0; i < maxknock; i++)
	{
		tempknocktime[i] = map(tempknocktime[i], 0, maxknocktime, 0, 100); //映射到0-100
		timediff = abs(tempknocktime[i] - rightknocktime[i]);
		if (timediff > 35)
		{
			return 0;
		}
		totaltimediff += timediff;
	}
	if (totaltimediff / knockcount > 20)
	{
		return 0;
	}
	return 1;
}

//programbutton
void reprogram()
{
	int i;
	int knockcount = 0;
	int maxknocktime = 0;
	while (1)
	{
		yellowLED = 1;
		redLED = 1;
		if (programbutton == 1)
			break;
		if (firstknock == 0)
		{
			if (key == 0)
			{
				delay(50);
				//if(key == 0)
				{
					listenknock();
				}
			}
		}
		if (firstknock == 1)
		{
			delay(50);
			//if(key == 0)
			{
				listenknock();
			}
		}
		if (endflag == 1)
		{
			endflag = 0;
			yellowLED = 0;
			redLED = 0;
			delay(200);
			for (i = 0; i < maxknock; i++)
			{
				if (tempknocktime[i] > 0)
				{
					knockcount++;
				}
				if (tempknocktime[i] > maxknocktime)
				{
					maxknocktime = tempknocktime[i];
				}
			}
			if (knockcount > 1 && knocknumber < 10)
			{
				for (i = 0; i < knockcount; i++)
				{
					rightknocktime[i] = map(tempknocktime[i], 0, maxknocktime, 0, 100);
				}
				for (i = 0; i < knockcount; i++)
				{
					yellowLED = 1;
					redLED = 1;
					blueLED = 1;
					delay(200);
					yellowLED = 0;
					redLED = 0;
					blueLED = 0;
					delay(200);
				}
				blueLED = 1;
				while (programbutton == 0)
					;
				break;
			}
			else
			{
				for (i = 0; i < 5; i++)
				{
					redLED = 1;
					delay(200);
					redLED = 0;
					delay(200);
				}
				for (i = 0; i < maxknock; i++)
				{
					tempknocktime[i] = 0;
				}
			}
		}
	}
}

//舵机
void motor()
{
	motornum = 0;
	motorturnflag = 0; //首次or重装初值
	motorstop = 0;
	TR1 = 1;
	while (motorstop < 100)
		;
	motorturnflag = 1;
	motorstop = 0;
	while (motorstop < 100)
		;
	TR1 = 0;
}

void main()
{
	int i, check;
	initial_main();
	while (1)
	{
		if (programbutton == 0)
		{
			reprogram();
			for (i = 0; i < maxknock; i++)
			{
				tempknocktime[i] = 0;
			}
			blueLED = 0;
			yellowLED = 0;
			redLED = 0;
		}
		if (firstknock == 0)
		{
			if (key == 0)
			{
				delay(50);
				//if(key == 0)
				{
					listenknock();
				}
			}
		}
		if (firstknock == 1)
		{
			delay(50);
			//if(key == 0)
			{
				listenknock();
			}
		}
		if (endflag == 1)
		{
			check = checkknocktime(); //判断正确返回1
			for (i = 0; i < maxknock; i++)
			{
				tempknocktime[i] = 0;
			}
			if (check == 1)
			{
				for (i = 0; i < 5; i++)
				{
					yellowLED = 1;
					delay(200);
					yellowLED = 0;
					delay(200);
				}
				motor(); //舵机
			}
			if (check == 0)
			{
				for (i = 0; i < 5; i++)
				{
					redLED = 1;
					delay(200);
					redLED = 0;
					delay(200);
				}
			}
		}
	}
}

//T0定时器中断
void timer0_count() interrupt 1
{
	//TH0 = 0xd8; //重装初值
	//TL0 = 0xf0;
	TH0 = 0xfc;
	TL0 = 0x18;
	num++;
}

//T1定时中断
void timer1_count() interrupt 3
{
	TH1 = 0xfe;
	TL1 = 0x0c;
	motornum++;
	if (motorturnflag == 0)
	{
		if (motornum < 5) //2.0ms
		{
			pwm = 1;
		}
		else
		{
			pwm = 0;
			if (motornum == 41) //18ms
			{
				motornum = 0;
				motorstop++;
			}
		}
	}
	else
	{
		if (motornum < 2) //0.5ms
		{
			pwm = 1;
		}
		else
		{
			pwm = 0;
			if (motornum == 41) //19.5ms
			{
				motornum = 0;
				motorstop++;
			}
		}
	}
}
