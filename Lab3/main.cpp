#include <dos.h> 

#define BUFF_WIDTH 80
#define CENTER_OFFSET 21
#define LEFT_OFFSET 10
#define REG_SCREEN_SIZE 9

struct VIDEO
{
	unsigned char symb;
	unsigned char attr;
};

int attribute = 0x5e;	//color

void print(int offset, int value);
void getRegisterValue();

void interrupt(*oldHandle68) (...);
void interrupt(*oldHandle69) (...);
void interrupt(*oldHandle6A) (...);
void interrupt(*oldHandle6B) (...);
void interrupt(*oldHandle6C) (...);
void interrupt(*oldHandle6D) (...);
void interrupt(*oldHandle6E) (...);
void interrupt(*oldHandle6F) (...);

void interrupt(*oldHandle08) (...);
void interrupt(*oldHandle09) (...);
void interrupt(*oldHandle0A) (...);
void interrupt(*oldHandle0B) (...);
void interrupt(*oldHandle0C) (...);
void interrupt(*oldHandle0D) (...);
void interrupt(*oldHandle0E) (...);
void interrupt(*oldHandle0F) (...);

void interrupt newHandle68(...) { getRegisterValue(); oldHandle68(); }
void interrupt newHandle69(...) { attribute++; getRegisterValue(); oldHandle69(); }
void interrupt newHandle6A(...) { getRegisterValue(); oldHandle6A(); }
void interrupt newHandle6B(...) { getRegisterValue(); oldHandle6B(); }
void interrupt newHandle6C(...) { getRegisterValue(); oldHandle6C(); }
void interrupt newHandle6D(...) { getRegisterValue(); oldHandle6D(); }
void interrupt newHandle6E(...) { getRegisterValue(); oldHandle6E(); }
void interrupt newHandle6F(...) { getRegisterValue(); oldHandle6F(); }

void interrupt newHandle08(...) { getRegisterValue(); oldHandle08(); }
void interrupt newHandle09(...) { getRegisterValue(); oldHandle09(); }
void interrupt newHandle0A(...) { getRegisterValue(); oldHandle0A(); }
void interrupt newHandle0B(...) { getRegisterValue(); oldHandle0B(); }
void interrupt newHandle0C(...) { attribute++; getRegisterValue(); oldHandle0C(); }
void interrupt newHandle0D(...) { getRegisterValue(); oldHandle0D(); }
void interrupt newHandle0E(...) { getRegisterValue(); oldHandle0E(); }
void interrupt newHandle0F(...) { getRegisterValue(); oldHandle0F(); }

void print(int offset, int value)
{
	char temp;

	VIDEO far* screen = (VIDEO far *)MK_FP(0xB800, 0);
	screen += CENTER_OFFSET * BUFF_WIDTH + offset;

	for (int i = 7; i >= 0; i--)
	{    
		temp = value % 2;
		value /= 2;
		screen->symb = temp + '0';
		screen->attr = attribute;
		screen++;
	}
}

void getRegisterValue() 
{
	//master
	print(0 + LEFT_OFFSET, inp(0x21));//thee registry of masks

	outp(0x20, 0x0B);//signal             
	print(REG_SCREEN_SIZE + LEFT_OFFSET, inp(0x20));//the registry of service

	outp(0x20, 0x0A);             
	print(REG_SCREEN_SIZE * 2 + LEFT_OFFSET, inp(0x20));//the registry of request
    //slave         
	print(BUFF_WIDTH + LEFT_OFFSET, inp(0xA1));

	outp(0xA0, 0x0B);             
	print(BUFF_WIDTH + REG_SCREEN_SIZE + LEFT_OFFSET, inp(0xA0));

	outp(0xA0, 0x0A);             
	print(BUFF_WIDTH + REG_SCREEN_SIZE * 2 + LEFT_OFFSET, inp(0xA0));
}

void init()
{
	// IRQ0-7
	oldHandle68 = getvect(0x08);       
	oldHandle69 = getvect(0x09);        
	oldHandle6A = getvect(0x0A);		
	oldHandle6B = getvect(0x0B);	    
	oldHandle6C = getvect(0x0C);	   
	oldHandle6D = getvect(0x0D); 	   
	oldHandle6E = getvect(0x0E); 	    
	oldHandle6F = getvect(0x0F);	    

	// IRQ8-15
	oldHandle08 = getvect(0x70);		
	oldHandle09 = getvect(0x71);	    		
	oldHandle0A = getvect(0x72);	    
	oldHandle0B = getvect(0x73);		
	oldHandle0C = getvect(0x74);		
	oldHandle0D = getvect(0x75);		
	oldHandle0E = getvect(0x76);		
	oldHandle0F = getvect(0x77);		

	setvect(0x68, newHandle68);
	setvect(0x69, newHandle69);
	setvect(0x6A, newHandle6A);
	setvect(0x6B, newHandle6B);
	setvect(0x6C, newHandle6C);
	setvect(0x6D, newHandle6D);
	setvect(0x6E, newHandle6E);
	setvect(0x6F, newHandle6F);

	setvect(0x08, newHandle08);
	setvect(0x09, newHandle09);
	setvect(0x0A, newHandle0A);
	setvect(0x0B, newHandle0B);
	setvect(0x0C, newHandle0C);
	setvect(0x0D, newHandle0D);
	setvect(0x0E, newHandle0E);
	setvect(0x0F, newHandle0F); 

	_disable();

	//Master
	outp(0x20, 0x11); 
	outp(0x21, 0x68); 
	outp(0x21, 0x04); 
	outp(0x21, 0x01); 

	//Slave
	outp(0xA0, 0x11); 
	outp(0xA1, 0x08); 
	outp(0xA1, 0x02); 
	outp(0xA1, 0x01); 

	_enable();
}

int main()
{
	unsigned far *fp;

	init();

	FP_SEG(fp) = _psp;//получение сегмента
	FP_OFF(fp) = 0x2c;//память для резидентной программы(смещение) 
	_dos_freemem(*fp);///free msdos area

	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);//residental 0 код завершения
	return 0;
}
