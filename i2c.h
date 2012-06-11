#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long
sbit sda=P2^0;
sbit scl=P2^1;
sbit beep=P2^3;
sbit dula=P2^6;
sbit wela=P2^7;
sbit lcden=P3^4;
sbit lcdrs=P3^5;
uchar a;
void delay(uchar x)
{
	uchar a,b;
	for(a=x;a>0;a--)
	 for(b=100;b>0;b--);
}
void delay1()
{ ;; }
void start()  //I2C开始信号
{	
	sda=1;
	delay1();
	scl=1;
	delay1();
	sda=0;
	delay1();
}

void stop()   //I2C停止
{
	sda=0;
	delay1();
	scl=1;
	delay1();
	sda=1;
	delay1();
}

void respons()  //I2C应答
{
	uchar i;
	scl=1;
	delay1();
	while((sda==1)&&(i<250))i++;
	scl=0;
	delay1();
}

void i2c_init()
{
	sda=1;
	delay1();
	scl=1;
	delay1();
}

void write_byte(uchar date)
{
	uchar i,temp;
	temp=date;


	for(i=0;i<8;i++)
	{
		temp=temp<<1;
		scl=0;
	    delay1();
		sda=CY;
		delay1();
		scl=1;
		delay1();
	//	scl=0;
     //   delay1();
	}
	scl=0;
	delay1();
	sda=1;
	delay1();
}

uchar read_byte()
{
	uchar i,k;
	scl=0;
	delay1();
	sda=1;
	delay1();
	for(i=0;i<8;i++)
	{
		scl=1;
		delay1();	
		k=(k<<1)|sda;
		scl=0;
		delay1();	
	}
	return k;
}


void write_add(uchar address,uchar date)
{
	start();
	write_byte(0xa0);
	respons();
	write_byte(address);
	respons();
	write_byte(date);
	respons();
	stop();
}

uchar read_add(uchar address)
{
	uchar date;
	start();
	write_byte(0xa0);
	respons();
	write_byte(address);
	respons();
	start();
	write_byte(0xa1);
	respons();
	date=read_byte();
	stop();
	return date;
}



