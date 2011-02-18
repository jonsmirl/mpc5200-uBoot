#include <pmic.h>

void Delay(void)
{
	unsigned long i,j;
	for(i=0;i<DELAY;i++);
}

void SCLH_SDAH()
{
	IIC_ESCL_Hi;
	IIC_ESDA_Hi;
	Delay();
}

void SCLH_SDAL()
{
	IIC_ESCL_Hi;
	IIC_ESDA_Lo;
	Delay();
}

void SCLL_SDAH()
{
	IIC_ESCL_Lo;
	IIC_ESDA_Hi;
	Delay();
}

void SCLL_SDAL()
{
	IIC_ESCL_Lo;
	IIC_ESDA_Lo;
	Delay();
}

void IIC_ELow()
{
	SCLL_SDAL();
	SCLH_SDAL();
	SCLH_SDAL();
	SCLL_SDAL();
}

void IIC_EHigh()
{
	SCLL_SDAH();
	SCLH_SDAH();
	SCLH_SDAH();
	SCLL_SDAH();
}

void IIC_EStart()
{
	SCLH_SDAH();
	SCLH_SDAL();
	Delay();
	SCLL_SDAL();
}

void IIC_EEnd()
{
	SCLL_SDAL();
	SCLH_SDAL();
	Delay();
	SCLH_SDAH();
}

void IIC_EAck()
{
	unsigned long ack;

	IIC_ESDA_INP;			// Function <- Input

	IIC_ESCL_Lo;
	Delay();
	IIC_ESCL_Hi;
	Delay();
	ack = GPD1DAT;
	IIC_ESCL_Hi;
	Delay();
	IIC_ESCL_Hi;
	Delay();

	IIC_ESDA_OUTP;			// Function <- Output (SDA)

	ack = (ack>>4)&0x1;
	while(ack!=0);

	SCLL_SDAL();
}

void IIC_ESetport(void)
{
	GPD1PUD &= ~(0xf<<8);	// Pull Up/Down Disable	SCL, SDA

	IIC_ESCL_Hi;
	IIC_ESDA_Hi;

	IIC_ESCL_OUTP;		// Function <- Output (SCL)
	IIC_ESDA_OUTP;		// Function <- Output (SDA)

	Delay();
}

void IIC_EWrite (unsigned char ChipId, unsigned char IicAddr, unsigned char IicData)
{
	unsigned long i;

	IIC_EStart();

////////////////// write chip id //////////////////
	for(i = 7; i>0; i--)
	{
		if((ChipId >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_ELow();	// write 'W'

	IIC_EAck();	// ACK

////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicAddr >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK

////////////////// write reg. data. //////////////////
	for(i = 8; i>0; i--)
	{
		if((IicData >> (i-1)) & 0x0001)
			IIC_EHigh();
		else
			IIC_ELow();
	}

	IIC_EAck();	// ACK

	IIC_EEnd();
}

void PMIC_InitIp(void)
{
	IIC_ESetport();

	IIC_EWrite(MAX8698_ADDR, 0x00, 0x3E);
	IIC_EWrite(MAX8698_ADDR, 0x01, 0xF0);

	// VDD_ARM
#if defined(CONFIG_CLK_1000_200_166_133)
	IIC_EWrite(MAX8698_ADDR, 0x04, 0xAA);	// default:0x99=>1.2v, 0xAA=>1.25v
	IIC_EWrite(MAX8698_ADDR, 0x05, 0xAA);	// default:0x99=>1.2v, 0xAA=>1.25v
#else
	IIC_EWrite(MAX8698_ADDR, 0x04, 0x99);	// default:0x99=>1.2v, 0x99=>1.2v
	IIC_EWrite(MAX8698_ADDR, 0x05, 0x99);	// default:0x99=>1.2v, 0x99=>1.2v
#endif
	// VDD_INT
	IIC_EWrite(MAX8698_ADDR, 0x06, 0x77);	// default:0x99=>1.2v, 0x77=>1.1v

	IIC_EWrite(MAX8698_ADDR, 0x07, 0x02);

	IIC_EWrite(MAX8698_ADDR, 0x08, 0x66);
	IIC_EWrite(MAX8698_ADDR, 0x09, 0x02);
	IIC_EWrite(MAX8698_ADDR, 0x0A, 0x0C);
	IIC_EWrite(MAX8698_ADDR, 0x0B, 0x0A);
	IIC_EWrite(MAX8698_ADDR, 0x0C, 0x0E);
	IIC_EWrite(MAX8698_ADDR, 0x0D, 0x33);
	IIC_EWrite(MAX8698_ADDR, 0x0E, 0x0E);
}

