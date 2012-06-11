#include<reg52.h>
#include<i2c.h>
#include<lcd.h>
//定义要写入LCD的字符
uchar code enter[]="Please enter:";
uchar code error[]="ERROR!";
uchar code ulock[]="Unlocked!";
uchar code inputoldpsw[]="Old Password:";
uchar code inputnewpsw[]="New Password:";
uchar code retypeerror[]="Different!";
uchar code pswchanged[]="Password Changed!";
uchar code retypenewpsw[]="Retype Password:";
uchar code smssend[]="SMS alert!";
uchar psw_mod,psw_mark,psw_count,flag,re_count,error_count,temp;
ulong psw_default,psw_get,psw_1,psw_2,psw_3,first_psw,sec_psw,psw_tmp,num_get;
//滴滴声报警
void didi() 
{
	beep=0;
	delay(50);
	beep=1;
	delay(100);
	beep=0;
	delay(50);
	beep=1;
}
//密码存储EEPROM
void psw_save(ulong psw){
	psw_1=psw%255;
	psw_2=psw/255%255;
	psw_3=psw/255/255;
	i2c_init();
	write_add(1,psw_1);
	delay(10);
	write_add(2,psw_2);
	delay(10);
	write_add(3,psw_3);
	delay(10);	
}
//密码读取EEPROM
ulong  psw_read(){
	i2c_init();
	psw_1=read_add(1);
	delay(10);
	psw_2=read_add(2);
	delay(10);
	psw_3=read_add(3);
	delay(10);
	psw_get=(psw_3)*255*255+(psw_2)*255+psw_1;
	return psw_get;
}
//密码状态存储函数
void psw_mod_save(uchar psw_mark){
		i2c_init();
		write_add(20,psw_mark);
		delay(10);
}
//密码状态查询函数
uchar psw_mod_read(){
	i2c_init();
	psw_mod=read_add(20);
	delay(10);
	return psw_mod;
}
//初始化函数
void init(){
		//获取psw_mod
		psw_mod=psw_mod_read();
		//判断是否为烧录后第一次,仅第一次初始化密码
		if(psw_mod!=psw_mark){
			psw_mod_save(psw_mark);
			psw_save(psw_default);
		}
}
void SerialInit()//初始化程序（必须使用，否则无法收发）
{
	
	TMOD=0x20;//设置定时器工作方式为8位自动装入数据
	TH1=0xfd;//装入初值，波特率9600
	TL1=0xfd;
	TR1=1;//打开定时器
	SM0=0;//设置串行通讯工作模式，（10为一部发送，波特率可变，由定时器1的溢出率控制）
	SM1=1;//(同上)在此模式下，定时器溢出一次就发送一个位的数据
	REN=1;//串行接收允许位（要先设置sm0sm1再开串行允许）
}

//串行口连续发送char型数组，遇到终止号/0将停止
void SerialSendChars(char *str)
{
	while(*str!='\0')
	{
		SBUF=*str;
		while((!TI));//等待发送完成信号（TI=1）出现
		TI=0;
		str++;
	}
}

void sms_alert()
{
	SerialInit();//串口初始化
	//多次初始化保证sim300 GSM模块初始化
	SerialSendChars("ati\r");//ati初始化sim300模块
	delay(200);
	SerialSendChars("ati\r");//ati初始化sim300模块
	delay(200);
	SerialSendChars("ati\r");//ati初始化sim300模块
	delay(200);

	//将短信息格式设为TEXT 模式
	SerialSendChars("AT+CMGF=1\r");
	delay(200);
	//设置字符格式为UCS2 模式
	SerialSendChars("AT+CSCS=\"UCS2\"\r");
	delay(200);
	//设置短消息发送相关参数
	SerialSendChars("AT+CSMP=17,167,0,24\r");
	delay(200);
	//设置手机号码unicode编码 //软件转换  15165337642
	SerialSendChars("AT+CMGS=\"00310035003100360035003300330037003600340032\"\r");
	//发送短信内容编码 //软件转换  //短信报警：密码锁自动报警，密码输入错误超过三次，请注意！
	SerialSendChars("77ED4FE18B6662A5FF1A5BC67801950181EA52A862A58B66FF0C5BC678018F93516595198BEF8D858FC74E096B21FF0C8BF76CE8610FFF01");
	SBUF=0x1A; //串口发送短信指令结束符
	while(!TI);//等待发送完成信号（TI=1）出现
	TI=0;
	delay(200);
}

//主函数
void main()
{
	//初始密码
	psw_default=123456;
	//初始密码状态标号(每次烧录修改为不同)8bit
	psw_mark=0x23;
	//初始化//写入初始密码和初始化标志锁
	//临时密码计数 密码输入位数计数
	psw_count=0;
	//错误次数计数
	error_count=0;
	//记录修改密码时候的输入密码次数
	re_count=0;
	//键盘输入状态标识
	flag=0;
	//初始化
	init();
	
	//lcd初始化 屏幕显示 please enter
	lcd_init();
	for(num=0;num<13;num++)
	{
		write_data(enter[num]);
		delay(5);
	}
	//改变光标位置
	write_com(0x80+0x40);
	//关闭光标和闪烁
	//write_com(0x0c);
	while(1){
		//键盘扫描
		P1=0xfe;
		temp=P1;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			delay(5);
			temp=P1;
			temp=temp&0xf0;
			if(temp!=0xf0)
			{
				switch(temp)
				{
					case 0x70: num_get=1; 
						break;	
					case 0xb0: num_get=2; 
						break;
					case 0xd0: num_get=3; 
						break;	
					case 0xe0: num_get=4; 
						break;
				}
			
			}
			
		}
		P1=0xfd;
		temp=P1;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			delay(5);
			temp=P1;
			temp=temp&0xf0;
			if(temp!=0xf0)
			{
				switch(temp)
				{
					case 0x70 : num_get=5; 
						break;	
					case 0xb0 : num_get=6; 
						break;
					case 0xd0 : num_get=7; 
						break;	
					case 0xe0 : num_get=8; 
						break;
				}
				
	
			}
		}
		P1=0xfb;
		temp=P1;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			delay(5);
			temp=P1;
			temp=temp&0xf0;
			if(temp!=0xf0)
			{
				switch(temp)
				{
					case 0x70 : num_get=9; 
						break;	
					case 0xb0 : num_get=10; 
						break;
					case 0xd0 : num_get=11; 
						break;	
					case 0xe0 : num_get=12; 
						break;
				}
	
			}
		}
		P1=0xf7;
		temp=P1;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			delay(5);
			temp=P1;
			temp=temp&0xf0;
			if(temp!=0xf0)
			{
				switch(temp)
				{
					case 0x70 : num_get=13; 
						break;	
					case 0xb0 : num_get=14; 
						break;
					case 0xd0 : num_get=15; 
						break;	
					case 0xe0 : num_get=16; 
						break;
				}
	
			}
		}
		//键盘扫描结束
		//判断对应操作				
		if(num_get==11){

			//恢复到输入密码的验证状态
			lcd_init();
			for(num=0;num<13;num++)
			{
				write_data(enter[num]);
				delay(5);
			}
			//光标定位
			write_com(0x80+0x40);
			//num_get清空
			num_get=0x00;
			//重置键盘输入状态flag=0 //验证输入
			flag=0;
			//重置密码输入位数计数
			psw_count=0;	
			//重置临时密码
			psw_tmp=0x00;
			
		}else if(num_get==12){

			//显示重置密码的提示
			lcd_init();
			for(num=0;num<13;num++)
			{
				write_data(inputoldpsw[num]);
				delay(5);
			}
			//光标定位
			write_com(0x80+0x40);
			//num_get清空
			num_get=0x00;
			//重置键盘输入状态flag=1 //修改密码前验证密码
			flag=1;
			//重置密码输入位数计数
			psw_count=0;	
			//重置临时密码
			psw_tmp=0x00;

		}else if(num_get>0&&num_get<11) {
			if(num_get==10){
				num_get=0;
			}
			psw_tmp=psw_tmp*10+num_get;
			psw_count++;
			if(psw_count>=6){
				if(flag==0){
					if(psw_tmp==psw_read()){
						//执行认证成功以后的动作 //认证正确点亮LED闪一下
						P1=0x00;
						for(num=1;num<10;num++){
							delay(1000);
						}
						P1=0xff;

						//显示输入正确后的提示
						lcd_init();
						for(num=0;num<13;num++)
						{
							write_data(enter[num]);
							delay(5);
						}
						//光标定位
						write_com(0x80+0x40);
						//认证成功提示
						for(num=0;num<9;num++)
						{
							write_data(ulock[num]);
							delay(5);
						}
						//显示UNLOCKED时候 光标隐藏并停止闪烁
						write_com(0x0C);
						//延时
						for(num=0;num<100;num++){
							delay(100);
						}
						//返回到密码输入状态
						lcd_init();
						for(num=0;num<13;num++)
						{
							write_data(enter[num]);
							delay(5);
						}
						//光标定位
						write_com(0x80+0x40);
						//设置光标闪烁
						write_com(0x0f);
						//重置错误统计次数
						error_count=0;
						//重置密码输入位数计数
						psw_count=0;	
						//重置临时密码
						psw_tmp=0x00;
					}else{
							//显示输入密码错误后的提示
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(enter[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							//错误提示error
							for(num=0;num<6;num++)
							{
								write_data(error[num]);
								delay(5);
							}
							//显示error时候 光标隐藏并停止闪烁
							write_com(0x0C);
							//延时2s提示并跳转
							for(num=0;num<10;num++){
								delay(100);
								didi();
								didi();
							}
							//返回到密码输入状态
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(enter[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							//设置光标闪烁
							write_com(0x0f);
							error_count++;
	
						}
						delay(1);
						if(error_count>=3){
							//错误三次后的严重错误警告
							//短信报警
							//提示短信报警状态
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(enter[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							for(num=0;num<10;num++)
							{
								write_data(smssend[num]);
								delay(5);
							}
							//光标不闪烁不显示
							write_com(0x0C);
							//LED常亮至报警结束
							P1=0x00;
							for(num=1;num<50;num++){
								delay(100);
								didi();
								didi();
							}
							//smsalert
							sms_alert();
							//延时1s提示并跳转
							for(num=1;num<50;num++){
								delay(100);
							}
							//返回到密码输入
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(enter[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							//设置光标闪烁
							write_com(0x0f);
							//错误计数清除
							error_count=0;
						}
						
					}else if(flag==1){
						//flag为1，则修改密码时修改密码
						if(psw_tmp==psw_read()){
							//显示输入正确后的提示
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(inputnewpsw[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							//下面处理第二次输入
						  	flag=3;
							//重置错误统计次数
							error_count=0;
							//重置密码输入位数计数
							psw_count=0;	
							//重置临时密码
							psw_tmp=0x00;
							

						}else{
							//修改密码验证失败的时候
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(inputoldpsw[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							//错误提示error
							for(num=0;num<6;num++)
							{
								write_data(error[num]);
								delay(5);
							}
							//显示error时候 光标隐藏并停止闪烁
							write_com(0x0C);
							//LED常亮至报警结束
							P1=0x00;
							//延时2s提示并跳转
							for(num=0;num<10;num++){
								delay(100);
								didi();
								didi();
							}
							//返回到密码输入状态
							lcd_init();
							for(num=0;num<13;num++)
							{
								write_data(inputoldpsw[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							//设置光标闪烁
							write_com(0x0f);
							error_count++;
							delay(1);
							if(error_count>=3){
								//错误三次后的严重错误警告
								//短信报警
								//提示短信报警状态
								lcd_init();
								for(num=0;num<13;num++)
								{
									write_data(inputoldpsw[num]);
									delay(5);
								}
								//光标定位
								write_com(0x80+0x40);
								//sms_alert
								for(num=0;num<10;num++)
								{
									write_data(smssend[num]);
									delay(5);
								}
								//光标不闪烁不显示
								write_com(0x0C);
								for(num=0;num<50;num++){
									delay(100);
									didi();
									didi();
								}
								//smsalert
								sms_alert();
								//延时1s提示并跳转
								for(num=1;num<50;num++){
									delay(100);
								}
								//返回到密码输入
								lcd_init();
								for(num=0;num<13;num++)
								{
									write_data(inputoldpsw[num]);
									delay(5);
								}
								//光标定位
								write_com(0x80+0x40);
								//设置光标闪烁
								write_com(0x0f);
								//错误计数清除
								error_count=0;
							}
						}
	
					}else if(flag==3){ //满6次 解锁和修改密码情况完毕 下面处理输入修改密码的情况
						if(re_count==0){
							//第一次进入时赋值给first_psw,并标志位re_count=1；
							first_psw = psw_tmp;//		first_psw,sec_psw	
							re_count=1;
							//跳到第二次输入确认的界面
							lcd_init();
							for(num=0;num<16;num++)
							{
								write_data(retypenewpsw[num]);
								delay(5);
							}
							//光标定位
							write_com(0x80+0x40);
							
						}else{
							sec_psw = psw_tmp;
							if(first_psw==sec_psw){
								//比对密码是否一致//进行存储
								psw_save(sec_psw);
								//显示存储成功的信息
								lcd_init();
								for(num=0;num<16;num++)
								{
									write_data(retypenewpsw[num]);
									delay(5);
								}
								//光标定位
								write_com(0x80+0x40);
								for(num=0;num<17;num++)
								{
									write_data(pswchanged[num]);
									delay(5);
								}
								//光标隐藏并停止闪烁
								write_com(0x0C);
								//延时以显示密码修改成功
								for(num=0;num<100;num++){
									delay(100);
								}
								
								//恢复到输入密码的验证状态
								lcd_init();
								for(num=0;num<13;num++)
								{
									write_data(enter[num]);
									delay(5);
								}
								//光标定位
								write_com(0x80+0x40);
								//num_get清空
								num_get=0x00;
								//重置键盘输入状态flag=0 //验证输入
								flag=0;
								//重置密码输入位数计数
								psw_count=0;	
								//重置临时密码
								psw_tmp=0x00;

							}else{
								//显示密码不一致的错误信息
								lcd_init();
								for(num=0;num<10;num++)
								{
									write_data(retypeerror[num]);
									delay(5);
								}
								//延时以显示错误信息
								for(num=0;num<10;num++){
									delay(100);
								}
								//跳转到输入密码
								lcd_init();
								for(num=0;num<13;num++)
								{
									write_data(inputnewpsw[num]);
									delay(5);
								}
								//光标定位
								write_com(0x80+0x40);
								//设置标志  下次输入再次进入重复输入修改密码
								flag=3;
								first_psw=0;
								sec_psw=0;
								re_count=0;

							}
						}
						
					}
				//重置密码输入位数计数
				psw_count=0;	
				//重置临时密码
				psw_tmp=0x00;
				}else{
					//如果输入不满6次，则按照输入显示*
					write_data('*');
				}
				//清空键值
				num_get=0x00;
			}
		//清除异常按键键值
		num_get=0x00;
	};

}


