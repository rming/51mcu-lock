
uchar num;

void write_com(uchar com)
{
	lcdrs=0;
	P0=com;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;
}

void write_data(uchar date)
{
	lcdrs=1;
	P0=date;
	delay(5);
	lcden=1;
	delay(5);
	lcden=0;
}
void lcd_init()
{
	dula=0;
	wela=0;
	lcden=0;
	write_com(0x38);
	//光标闪烁并打开显示
	write_com(0x0f);

	write_com(0x06);
	write_com(0x01);
	write_com(0x80);
}


