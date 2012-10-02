#include <math.h>
#include "inc/lm3s9790.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"

// Ket thuc chi dan khai bao tan so

/************************************************************************************/

unsigned char glcd_buff[1024];
unsigned int glcd_address;
unsigned char glcd_column = 0;
unsigned char glcd_line = 0;

const char basic_font[] =
{
	0x00,0x00,0x00,0x00,0x00,		// Code for  
	0x00,0x00,0x4F,0x00,0x00,		// Code for !
	0x00,0x07,0x00,0x07,0x00,		// Code for "
	0x14,0x7F,0x14,0x7F,0x14,		// Code for #
	0x24,0x2A,0x7F,0x2A,0x12,		// Code for $
	0x23,0x13,0x08,0x64,0x62,		// Code for %
	0x36,0x49,0x55,0x22,0x50,		// Code for &
	0x00,0x05,0x03,0x00,0x00,		// Code for '
	0x00,0x1C,0x22,0x41,0x00,		// Code for (
	0x00,0x41,0x22,0x1C,0x00,		// Code for )
	0x14,0x08,0x3E,0x08,0x14,		// Code for *
	0x08,0x08,0x3E,0x08,0x08,		// Code for +
	0x00,0x50,0x30,0x00,0x00,		// Code for ,
	0x08,0x08,0x08,0x08,0x08,		// Code for -
	0x00,0x60,0x60,0x00,0x00,		// Code for .
	0x20,0x10,0x08,0x04,0x02,		// Code for /
	0x3E,0x51,0x49,0x45,0x3E,		// Code for 0
	0x00,0x42,0x7F,0x40,0x00,		// Code for 1
	0x42,0x61,0x51,0x49,0x46,		// Code for 2
	0x21,0x41,0x45,0x4B,0x31,		// Code for 3
	0x18,0x14,0x12,0x7F,0x10,		// Code for 4
	0x27,0x45,0x45,0x45,0x39,		// Code for 5
	0x3C,0x4A,0x49,0x49,0x30,		// Code for 6
	0x01,0x71,0x09,0x05,0x03,		// Code for 7
	0x36,0x49,0x49,0x49,0x36,		// Code for 8
	0x06,0x49,0x49,0x29,0x1E,		// Code for 9
	0x00,0x36,0x36,0x00,0x00,		// Code for :
	0x00,0x56,0x36,0x00,0x00,		// Code for ;
	0x00,0x08,0x14,0x22,0x41,		// Code for <
	0x14,0x14,0x14,0x14,0x14,		// Code for =
	0x41,0x22,0x14,0x08,0x00,		// Code for >
	0x02,0x01,0x59,0x05,0x02,		// Code for ?
	0x32,0x49,0x79,0x41,0x3E,		// Code for @
	0x7E,0x11,0x11,0x11,0x7E,		// Code for A
	0x7F,0x49,0x49,0x49,0x36,		// Code for B
	0x3E,0x41,0x41,0x41,0x22,		// Code for C
	0x7F,0x41,0x41,0x22,0x1C,		// Code for D
	0x7F,0x49,0x49,0x49,0x41,		// Code for E
	0x7F,0x09,0x09,0x09,0x01,		// Code for F
	0x3E,0x41,0x41,0x49,0x7A,		// Code for G
	0x7F,0x08,0x08,0x08,0x7F,		// Code for H
	0x00,0x41,0x7F,0x41,0x00,		// Code for I
	0x20,0x40,0x41,0x3F,0x01,		// Code for J
	0x7F,0x08,0x14,0x22,0x41,		// Code for K
	0x7F,0x40,0x40,0x40,0x40,		// Code for L
	0x7F,0x02,0x0C,0x02,0x7F,		// Code for M
	0x7F,0x04,0x08,0x10,0x7F,		// Code for N
	0x3E,0x41,0x41,0x41,0x3E,		// Code for O
	0x7F,0x09,0x09,0x09,0x06,		// Code for P
	0x3E,0x41,0x51,0x21,0x5E,		// Code for Q
	0x7F,0x09,0x19,0x29,0x46,		// Code for R
	0x46,0x49,0x49,0x49,0x31,		// Code for S
	0x01,0x01,0x7F,0x01,0x01,		// Code for T
	0x3F,0x40,0x40,0x40,0x3F,		// Code for U
	0x1F,0x20,0x40,0x20,0x1F,		// Code for V
	0x3F,0x40,0x30,0x40,0x3F,		// Code for W
	0x63,0x14,0x08,0x14,0x63,		// Code for X
	0x07,0x08,0x70,0x08,0x07,		// Code for Y
	0x61,0x51,0x49,0x45,0x43,		// Code for Z
	0x00,0x7F,0x41,0x41,0x00,		// Code for [
	0x02,0x04,0x08,0x10,0x20,		// Code for "\"
	0x00,0x41,0x41,0x7F,0x00,		// Code for ]
	0x04,0x02,0x01,0x02,0x04,		// Code for ^
	0x40,0x40,0x40,0x40,0x40,		// Code for _
	0x00,0x01,0x02,0x04,0x00,		// Code for `
	0x20,0x54,0x54,0x54,0x78,		// Code for a
	0x7F,0x48,0x44,0x44,0x38,		// Code for b
	0x38,0x44,0x44,0x44,0x20,		// Code for c
	0x38,0x44,0x44,0x48,0x7F,		// Code for d
	0x38,0x54,0x54,0x54,0x18,		// Code for e
	0x08,0x7E,0x09,0x01,0x02,		// Code for f
	0x0C,0x52,0x52,0x52,0x3E,		// Code for g
	0x7F,0x08,0x04,0x04,0x78,		// Code for h
	0x00,0x44,0x7D,0x40,0x00,		// Code for i
	0x20,0x40,0x44,0x3D,0x00,		// Code for j
	0x7F,0x10,0x28,0x44,0x00,		// Code for k
	0x00,0x41,0x7F,0x40,0x00,		// Code for l
	0x7C,0x04,0x7C,0x04,0x78,		// Code for m
	0x7C,0x08,0x04,0x04,0x78,		// Code for n
	0x38,0x44,0x44,0x44,0x38,		// Code for o
	0x7C,0x14,0x14,0x14,0x08,		// Code for p
	0x08,0x14,0x14,0x14,0x7C,		// Code for q
	0x7C,0x08,0x04,0x04,0x08,		// Code for r
	0x48,0x54,0x54,0x54,0x20,		// Code for s
	0x04,0x3F,0x44,0x40,0x20,		// Code for t
	0x3C,0x40,0x40,0x20,0x7C,		// Code for u
	0x1C,0x20,0x40,0x20,0x1C,		// Code for v
	0x3C,0x40,0x30,0x40,0x3C,		// Code for w
	0x44,0x28,0x10,0x28,0x44,		// Code for x
	0x0C,0x50,0x50,0x50,0x3C,		// Code for y
	0x44,0x64,0x54,0x4C,0x44,		// Code for z
	0x00,0x08,0x36,0x41,0x00,		// Code for {
	0x00,0x00,0x7F,0x00,0x00,		// Code for |
	0x00,0x41,0x36,0x08,0x00,		// Code for }
	0x10,0x08,0x10,0x08,0x00 		// Code for ~
};

//**************************************************************************************************
//
//Set up PORT PIN compatiably with GLCD
//
//**************************************************************************************************
#define EN_PIN   		GPIO_PIN_7
#define EN_PORT   		GPIO_PORTG_BASE
#define EN_PERIPH  		SYSCTL_PERIPH_GPIOG

#define DI_PIN		  	GPIO_PIN_5
#define DI_PORT		  	GPIO_PORTF_BASE
#define DI_PERIPH  		SYSCTL_PERIPH_GPIOF

#define RW_PIN		  	GPIO_PIN_1
#define RW_PORT		  	GPIO_PORTG_BASE
#define RW_PERIPH  		SYSCTL_PERIPH_GPIOG

#define RST_PIN			GPIO_PIN_2
#define RST_PORT		GPIO_PORTJ_BASE
#define RST_PERIPH  	SYSCTL_PERIPH_GPIOJ

#define CS1_PIN			GPIO_PIN_0
#define CS1_PORT		GPIO_PORTJ_BASE
#define CS1_PERIPH  	SYSCTL_PERIPH_GPIOJ

#define CS2_PIN			GPIO_PIN_1
#define CS2_PORT		GPIO_PORTJ_BASE
#define CS2_PERIPH  	SYSCTL_PERIPH_GPIOJ

#define DATA_PERIPH	  	SYSCTL_PERIPH_GPIOH
#define DATA_PORT		GPIO_PORTH_BASE
#define DATA_DIR		GPIO_PORTH_DIR_R
#define DATA_DEN		GPIO_PORTH_DEN_R
#define DATA			GPIO_PORTH_DATA_R

/************************************************************************************/

// Cac function

void GLCD_INIT();
void GLCD_WAIT();
void GLCD_FILL(unsigned char);
void GLCD_OUT_DATA(unsigned char);
void GLCD_OUT_CMD(unsigned char);
void GLCD_LINE_CHANGE(unsigned char);
void GLCD_CS1_ENABLE();
void GLCD_CS2_ENABLE();
void GLCD_DISPLAY();
void delayTime (unsigned long);

// ----------------------------------------------------------------------------------

void GLCD_INIT()
{
	SysCtlPeripheralEnable(EN_PERIPH);
	SysCtlPeripheralEnable(RST_PERIPH);
	SysCtlPeripheralEnable(DI_PERIPH);
	SysCtlPeripheralEnable(RW_PERIPH);
	SysCtlPeripheralEnable(CS1_PERIPH);
	SysCtlPeripheralEnable(CS2_PERIPH);
	SysCtlPeripheralEnable(DATA_PERIPH);
	
	GPIOPinTypeGPIOOutput(EN_PORT,EN_PIN);
	GPIOPinTypeGPIOOutput(DI_PORT,DI_PIN);
	GPIOPinTypeGPIOOutput(RW_PORT,RW_PIN);
	GPIOPinTypeGPIOOutput(RST_PORT,RST_PIN);
	GPIOPinTypeGPIOOutput(CS1_PORT,CS1_PIN);
	GPIOPinTypeGPIOOutput(CS2_PORT,CS2_PIN);
	GPIOPinTypeGPIOOutput(DATA_PORT,GPIO_PIN_0 |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

	GPIOPinWrite(RST_PORT,RST_PIN,RST_PIN);
	GPIOPinWrite(CS1_PORT,CS1_PIN,CS1_PIN);
	GPIOPinWrite(CS2_PORT,CS2_PIN,CS2_PIN);
	GPIOPinWrite(RW_PORT,RW_PIN,~RW_PIN);

	delayTime(100000);
	GLCD_OUT_CMD(0xC0); 

	delayTime(100000);
	GLCD_OUT_CMD(0x40);

	delayTime(100000);
	GLCD_OUT_CMD(0xB8);

	delayTime(100000);
	GLCD_OUT_CMD(0x3F);

	GPIOPinWrite(CS1_PORT,CS1_PIN,~CS1_PIN);
	GPIOPinWrite(CS2_PORT,CS2_PIN,~CS2_PIN);
}

void GLCD_DISPLAY()
{
	for(glcd_address = 0; glcd_address < 1024; glcd_address++)
	{
		if(glcd_column == 128)
		{
			glcd_column = 0;
			glcd_line++;
			
			// Gui lenh tang line
			GPIOPinWrite(CS1_PORT,CS1_PIN,CS1_PIN);
			GPIOPinWrite(CS2_PORT,CS2_PIN,CS2_PIN);
	
			glcd_line &= 0x07;
			glcd_line |= 0xB8;

			GLCD_OUT_CMD(glcd_line);

			GLCD_OUT_CMD(0x40);
		} 

		if(glcd_column <= 63)
			GLCD_CS1_ENABLE();

		if(glcd_column >= 64)
			GLCD_CS2_ENABLE();

		GLCD_OUT_DATA(glcd_buff[glcd_address]);

		glcd_column++;
	}
}

void GLCD_CS1_ENABLE()
{
	GPIOPinWrite(CS1_PORT,CS1_PIN,CS1_PIN);
	GPIOPinWrite(CS2_PORT,CS2_PIN,~CS2_PIN);
}

void GLCD_CS2_ENABLE()
{
	GPIOPinWrite(CS1_PORT,CS1_PIN,~CS1_PIN);
	GPIOPinWrite(CS2_PORT,CS2_PIN,CS2_PIN);
}


void delayTime (unsigned long counter)
{
	while (counter--);
}

void GLCD_OUT_CMD(unsigned char glcd_cmd)
{
	GPIOPinWrite(DI_PORT,DI_PIN,~DI_PIN);
	GPIOPinWrite(EN_PORT,EN_PIN,EN_PIN);
	
	DATA = glcd_cmd;
	delayTime(100);
	DATA = glcd_cmd;
	delayTime(100);
	DATA = glcd_cmd;
	DATA = glcd_cmd;
	
	GPIOPinWrite(EN_PORT,EN_PIN,~EN_PIN);
}

void GLCD_OUT_DATA(unsigned char glcd_data)
{
	GPIOPinWrite(DI_PORT,DI_PIN,DI_PIN);
	GPIOPinWrite(EN_PORT,EN_PIN,EN_PIN);
	
	DATA = glcd_data;
	delayTime(100);
	DATA = glcd_data;
	delayTime(100);
	DATA = glcd_data;
	
	GPIOPinWrite(EN_PORT,EN_PIN,~EN_PIN);
}

// ----------------------------------------------------------------------------------

void GLCD_POINT_DRW(unsigned char x, unsigned char y)
{
	unsigned char h_line;
	unsigned int temp;
	temp = x + ((y >> 3) << 7);
	h_line = (unsigned char)(y & 0x07);
	glcd_buff[temp] |= (1<<h_line);
}

void GLCD_POINT_CLR(unsigned char x, unsigned char y)
{
	unsigned char h_line;
	unsigned int temp;
	temp = x + ((y >> 3) << 7);
	h_line = (unsigned char)(y & 0x07);
	glcd_buff[temp] &= ~(1<<h_line);
}

void GLCD_DRW_REC_EMPTY(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char black_white)
{
	unsigned char glcd_temp;
	if(x1 > x2)
	{
		glcd_temp = x1;
		x1 = x2;
		x2 = glcd_temp;
	}
	if(y1 > y2)
	{
		glcd_temp = y1;
		y1 = y2;
		y2 = glcd_temp;
	}
	if(black_white)
	{
		for(glcd_temp=x1;glcd_temp<=x2;glcd_temp++)
		{
			GLCD_POINT_DRW(glcd_temp,y1);
			GLCD_POINT_DRW(glcd_temp,y2);
		}
		for(glcd_temp=y1;glcd_temp<=y2;glcd_temp++)
		{
			GLCD_POINT_DRW(x1,glcd_temp);
			GLCD_POINT_DRW(x2,glcd_temp);
		}
	}
	else
	{
		for(glcd_temp=x1;glcd_temp<=x2;glcd_temp++)
		{
			GLCD_POINT_CLR(glcd_temp,y1);
			GLCD_POINT_CLR(glcd_temp,y2);
		}
		for(glcd_temp=y1;glcd_temp<=y2;glcd_temp++)
		{
			GLCD_POINT_CLR(x1,glcd_temp);
			GLCD_POINT_CLR(x2,glcd_temp);
		}
	}
}

void GLCD_DRW_REC_SOLID(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char black_white)
{
	unsigned char glcd_temp;
	unsigned char i,j;
	if(x1 > x2)
	{
		glcd_temp = x1;
		x1 = x2;
		x2 = glcd_temp;
	}
	if(y1 > y2)
	{
		glcd_temp = y1;
		y1 = y2;
		y2 = glcd_temp;
	}
	for(i=x1;i<=x2;i++)
	for(j=y1;j<=y2;j++)
	{
		if(black_white)
			GLCD_POINT_DRW(i,j);
		else
			GLCD_POINT_CLR(i,j);
	}
}

void GLCD_DRW_LINE(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char black_white)
{
	unsigned char i;
	float a, b;

	if(x2 == x1)
	{
		if(y2 >= y1)
			for(i=y1;i<=y2;i++)
				if(black_white != 0)
					GLCD_POINT_DRW(x1,i);
				else
					GLCD_POINT_CLR(x1,i);
		else
			for(i=y2;i<=y1;i++)
				if(black_white != 0)
					GLCD_POINT_DRW(x1,i);
				else
					GLCD_POINT_CLR(x1,i);
	}
	else
	{
		a = (float)(y2 - y1) / (float)(x2 - x1);
		b = (float)(y2 - a*x2);

		if(x2 > x1)
		{
			for(i=x1;i<=x2;i++)
				if(black_white != 0)
					GLCD_POINT_DRW(i,(a*i + b));
				else
					GLCD_POINT_CLR(i,(a*i + b));
		}
		else
		{
			for(i=x2;i<=x1;i++)
				if(black_white != 0)
					GLCD_POINT_DRW(i,(a*i + b));
				else
					GLCD_POINT_CLR(i,(a*i + b));
		}
	}
}

void GLCD_DRW_VECTOR(unsigned char x1, unsigned char y1, unsigned int angle, unsigned char length, unsigned char black_white)
{
	unsigned int i;
	char x_max;
	double a;
	double goc;
	if(angle == 0)
	{
		if(black_white != 0)
			GLCD_DRW_LINE(x1,y1,x1,y1-length,1);
		else
			GLCD_DRW_LINE(x1,y1,x1,y1-length,0);
	}
	else if(angle == 90)
	{
		if(black_white != 0)
			GLCD_DRW_LINE(x1,y1,x1+length,y1,1);
		else
			GLCD_DRW_LINE(x1,y1,x1+length,y1,0);
	}
	else if(angle == 180)
	{
		if(black_white != 0)
			GLCD_DRW_LINE(x1,y1,x1,y1+length,1);
		else
			GLCD_DRW_LINE(x1,y1,x1,y1+length,0);
	}
	else if(angle == 270)
	{
		if(black_white != 0)
			GLCD_DRW_LINE(x1,y1,x1-length,y1,1);
		else
			GLCD_DRW_LINE(x1,y1,x1-length,y1,0);
	}
	else
	{
		if(angle < 180)
			goc = ((double)(angle*3.14)/180);
		else
			goc = ((double)((360 - angle)*3.14)/180);
		
		x_max = (char)(length*sin(goc));

		a = 1/tan(goc);
		if(angle < 180)
			for(i=x1;i<=x1+x_max;i++)
			{
				if(black_white)
					GLCD_POINT_DRW(i,(unsigned char)(y1 - a*(i-x1)));
				else
					GLCD_POINT_CLR(i,(unsigned char)(y1 - a*(i-x1)));
			}
		else
			for(i=(x1-x_max);i<=x1;i++)
			{
				if(black_white)
					GLCD_POINT_DRW(i,(unsigned char)(y1 - a*(x1-i)));
				else
					GLCD_POINT_CLR(i,(unsigned char)(y1 - a*(x1-i)));
			}
	}
}

void GLCD_DRW_CIRCLE_EMPTY(unsigned char x, unsigned char y, unsigned char r, unsigned char black_white)
{
	unsigned char i,j;
	unsigned int r2,ri2;
	r2 = r*r;
	for(i=0;i<=r;i++)
	{
		j = 0;
		ri2 = (r-i)*(r-i);
		
		while((ri2 + (r-j)*(r-j)) > r2)
		{
			j++;
		}

		if(black_white)
		{
			GLCD_POINT_DRW(x-r+i,y-r+j);
			GLCD_POINT_DRW(x-r+i,y+r-j);
			GLCD_POINT_DRW(x+r-i,y+r-j);
			GLCD_POINT_DRW(x+r-i,y-r+j);
		}
		else
		{
			GLCD_POINT_CLR(x-r+i,y-r+j);
			GLCD_POINT_CLR(x-r+i,y+r-j);
			GLCD_POINT_CLR(x+r-i,y+r-j);
			GLCD_POINT_CLR(x+r-i,y-r+j);
		}
	}
}
void GLCD_DRW_CIRCLE_SOLID(unsigned char x, unsigned char y, unsigned char r, unsigned char black_white)
{
	unsigned char i,j;
	unsigned int r2,ri2;
	r2 = r*r;
	for(i=0;i<=r;i++)
	{
		ri2 = (r-i)*(r-i);
		for(j=0;j<=r;j++)
		{
			if((ri2 + (r-j)*(r-j)) <= r2)
			{
				if(black_white != 0)
				{
					GLCD_POINT_DRW(x-r+i,y-r+j);
					GLCD_POINT_DRW(x-r+i,y+r-j);
					GLCD_POINT_DRW(x+r-i,y+r-j);
					GLCD_POINT_DRW(x+r-i,y-r+j);
				}
				else
				{
					GLCD_POINT_CLR(x-r+i,y-r+j);
					GLCD_POINT_CLR(x-r+i,y+r-j);
					GLCD_POINT_CLR(x+r-i,y+r-j);
					GLCD_POINT_CLR(x+r-i,y-r+j);
				}
			}
		}
	}
}
void GLCD_FILL(unsigned char black_white)
{
	unsigned char i,j;

	for(i=0;i<=127;i++)
		for(j=0;j<=63;j++)
		{
			if(black_white)
				GLCD_POINT_DRW(i,j);
			else
				GLCD_POINT_CLR(i,j);
		}
}	

unsigned int GLCD_CHAR_SET(unsigned char glcd_x, unsigned char glcd_y, const char * glcd_font, unsigned char num)
{
	unsigned char i;
	unsigned char c;
	unsigned int add;

	add = (glcd_y / 8)*128 + glcd_x;
	glcd_y = glcd_y % 8;
	
	if((num >= 32)&&(num <= 126))
	{
		glcd_font = glcd_font + (num-32)*5;
		
		for(i=0;i<5;i++)
		{
			//c = pgm_read_byte (glcd_font);
			c = (unsigned char)glcd_font;
			glcd_buff[add] |= (c << glcd_y);
			glcd_buff[add + 128] |= (c >> (8 - glcd_y));
			add++;
			glcd_font++;
		}
	}
	return (glcd_x + 6);
}

unsigned int GLCD_CHAR_CLR(unsigned char glcd_x, unsigned char glcd_y, const char * glcd_font, unsigned char num)
{
	unsigned char i;
	unsigned char c;
	unsigned int add;

	add = (glcd_y / 8)*128 + glcd_x;
	glcd_y = glcd_y % 8;
	
	if((num >= 32)&&(num <= 126))
	{
		glcd_font = glcd_font + (num-32)*5;
		
		for(i=0;i<5;i++)
		{
			//c = pgm_read_byte (glcd_font);
			c = (unsigned short int)glcd_font;
			glcd_buff[add] &= ((c << glcd_y) ^ 0xFF);
			glcd_buff[add + 128] &= ((c >> (8 - glcd_y)) ^ 0xFF);
			add++;
			glcd_font++;
		}
	}
	return (glcd_x + 6);
}

unsigned int GLCD_OUT_STR(unsigned char glcd_x, unsigned char glcd_y, char *glcd_str, unsigned char black_white)
{
	while(*glcd_str)
	{
		if(black_white)
			GLCD_CHAR_SET(glcd_x, glcd_y, basic_font, *glcd_str);
		else
			GLCD_CHAR_CLR(glcd_x, glcd_y, basic_font, *glcd_str);
		glcd_x = glcd_x + 6;
		glcd_str++;
	}
	return (glcd_x);
}
/*
unsigned int GLCD_OUT_PGM(unsigned char glcd_x, unsigned char glcd_y, const char * glcd_ptr, unsigned char black_white)
{
	char glcd_temp;
	glcd_temp = pgm_read_byte(glcd_ptr++);
	if(black_white)
		while(glcd_temp)
		{
			GLCD_CHAR_SET(glcd_x, glcd_y, basic_font, glcd_temp);
			glcd_x = glcd_x + 6;
			glcd_temp = pgm_read_byte(glcd_ptr++);
		}
	else
		while(glcd_temp)
		{
			GLCD_CHAR_CLR(glcd_x, glcd_y, basic_font, glcd_temp);
			glcd_x = glcd_x + 6;
			glcd_temp = pgm_read_byte(glcd_ptr++);
		}
	return (glcd_x);
}
*/
unsigned int GLCD_OUT_DEC(unsigned char glcd_x, unsigned char glcd_y, unsigned long int glcd_n, unsigned char glcd_length, unsigned char black_white)
{
	unsigned char glcd_temp;
	unsigned char i;

	if(black_white)
		for(i=glcd_length;i>0;i--)
		{
			glcd_temp = (glcd_n % 10) + 0x30;
			glcd_n = glcd_n / 10;
			GLCD_CHAR_SET((glcd_x + (i-1)*6), glcd_y, basic_font, glcd_temp);
		}
	else
		for(i=glcd_length;i>0;i--)
		{
			glcd_temp = (glcd_n % 10) + 0x30;
			glcd_n = glcd_n / 10;
			GLCD_CHAR_CLR((glcd_x + (i-1)*6), glcd_y, basic_font, glcd_temp);
		}
	return (glcd_x + glcd_length*6);
}

void GLCD_IMAGE(unsigned char * image)
{
	int i;
	for(i=0;i<1024;i++)
		glcd_buff[i] = image[i];

}