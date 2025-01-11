U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);  
//定义了名为u8g2的对象，U8G2_R0表示使用默认的I2C地址，SDA 8, SCK 9

int L0_position = 1;
int L1_position = 1;
int L2_position = 1;
int L3_position = 1;

const static unsigned char QRcode[] PROGMEM ={0xFF,0x3F,0x0C,0x33,0xFC,0xF0,0xFF,0x03,0xFF,0x3F,0x0C,0x33,0xFC,0xF0,0xFF,0x03,0x03,0x30,0xF0,0xF3,0xF0,0x33,0x00,0x03,0x03,0x30,0xF0,0xF3,0xF0,0x33,0x00,0x03,0xF3,0x33,0x3F,0xC3,0xC0,0x33,0x3F,0x03,0xF3,0x33,0x3F,0xC3,0xC0,0x33,0x3F,0x03,0xF3,0x33,0xFC,0xF0,0xF0,0x30,0x3F,0x03,0xF3,0x33,0xFC,0xF0,0xF0,0x30,0x3F,0x03,0xF3,0x33,0x30,0x3C,0xFC,0x30,0x3F,0x03,0xF3,0x33,0x30,0x3C,0xFC,0x30,0x3F,0x03,0x03,0x30,0x0C,0xF0,0xFF,0x30,0x00,0x03,0x03,0x30,0x0C,0xF0,0xFF,0x30,0x00,0x03,0xFF,0x3F,0x33,0x33,0x33,0xF3,0xFF,0x03,0xFF,0x3F,0x33,0x33,0x33,0xF3,0xFF,0x03,0x00,0x00,0x3F,0xFF,0x3C,0x03,0x00,0x00,0x00,0x00,0x3F,0xFF,0x3C,0x03,0x00,0x00,0x3F,0xFF,0xF3,0x0C,0xFF,0x3F,0x30,0x00,0x3F,0xFF,0xF3,0x0C,0xFF,0x3F,0x30,0x00,0xCC,0x0F,0xF3,0xCC,0x03,0x3C,0x0C,0x03,0xCC,0x0F,0xF3,0xCC,0x03,0x3C,0x0C,0x03,0x0C,0xF0,0x0C,0x00,0x0C,0x30,0xF0,0x03,0x0C,0xF0,0x0C,0x00,0x0C,0x30,0xF0,0x03,0x3C,0x0C,0xC3,0xFC,0xFC,0x3C,0xC3,0x00,0x3C,0x0C,0xC3,0xFC,0xFC,0x3C,0xC3,0x00,0xFC,0x3C,0x00,0x03,0xCC,0x3F,0xCC,0x03,0xFC,0x3C,0x00,0x03,0xCC,0x3F,0xCC,0x03,0xC3,0x0C,0xCC,0x3F,0x3C,0xFF,0x0C,0x03,0xC3,0x0C,0xCC,0x3F,0x3C,0xFF,0x0C,0x03,0xCC,0xFF,0xFF,0x0F,0x0F,0xC3,0xCC,0x03,0xCC,0xFF,0xFF,0x0F,0x0F,0xC3,0xCC,0x03,0xFC,0x00,0x30,0x3F,0xCF,0x03,0xCC,0x00,0xFC,0x00,0x30,0x3F,0xCF,0x03,0xCC,0x00,0x00,0xFC,0xF3,0x00,0xCC,0xFF,0xCC,0x03,0x00,0xFC,0xF3,0x00,0xCC,0xFF,0xCC,0x03,0x3C,0x0C,0xCF,0xCC,0x03,0x3F,0x3C,0x03,0x3C,0x0C,0xCF,0xCC,0x03,0x3F,0x3C,0x03,0xC3,0x3F,0x03,0x00,0x33,0xCC,0xC0,0x03,0xC3,0x3F,0x03,0x00,0x33,0xCC,0xC0,0x03,0xCC,0xC0,0xCC,0x3C,0xFC,0x0C,0xCC,0x00,0xCC,0xC0,0xCC,0x3C,0xFC,0x0C,0xCC,0x00,0x33,0x33,0x0F,0x03,0xCF,0xFF,0x03,0x00,0x33,0x33,0x0F,0x03,0xCF,0xFF,0x03,0x00,0x00,0x00,0xCF,0xF3,0xCF,0x03,0xF3,0x03,0x00,0x00,0xCF,0xF3,0xCF,0x03,0xF3,0x03,0xFF,0x3F,0xFF,0x3F,0xC0,0x33,0xCF,0x03,0xFF,0x3F,0xFF,0x3F,0xC0,0x33,0xCF,0x03,0x03,0x30,0x03,0x0F,0x0F,0x03,0xCF,0x00,0x03,0x30,0x03,0x0F,0x0F,0x03,0xCF,0x00,0xF3,0x33,0xCF,0xC3,0x0F,0xFF,0xC3,0x00,0xF3,0x33,0xCF,0xC3,0x0F,0xFF,0xC3,0x00,0xF3,0x33,0xF0,0x3C,0xCC,0x03,0xF3,0x03,0xF3,0x33,0xF0,0x3C,0xCC,0x03,0xF3,0x03,0xF3,0x33,0x03,0x3C,0x00,0xC3,0x0F,0x03,0xF3,0x33,0x03,0x3C,0x00,0xC3,0x0F,0x03,0x03,0x30,0x3F,0x3F,0xFF,0x3F,0xC3,0x00,0x03,0x30,0x3F,0x3F,0xFF,0x3F,0xC3,0x00,0xFF,0x3F,0xF3,0xFF,0xCF,0xFF,0xCF,0x03,0xFF,0x3F,0xF3,0xFF,0xCF,0xFF,0xCF,0x03};


void L0P1(){
  L0_position = 1;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 0, 125, 17);  
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 15,"开始提问");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"设备设置");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"GPT模型设置");
  u8g2.setDrawColor(1);  
  u8g2.drawUTF8(0, 62,"使用教程");   
  u8g2.sendBuffer();  
}

void L0P2(){
  L0_position = 2;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 17, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"开始提问");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 31,"设备设置");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"GPT模型设置");
  u8g2.setDrawColor(1);  
  u8g2.drawUTF8(0, 62,"使用教程");  
  u8g2.sendBuffer();  
}

void L0P3(){
  L0_position = 3;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 33, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"开始提问");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"设备设置");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 47,"GPT模型设置");
  u8g2.setDrawColor(1);  
  u8g2.drawUTF8(0, 62,"使用教程");   
  u8g2.sendBuffer();  
}

void L0P4(){
  L0_position = 4;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 49, 125, 15);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"开始提问");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"设备设置");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"GPT模型设置");
  u8g2.setDrawColor(0);  
  u8g2.drawUTF8(0, 62,"使用教程");  
  u8g2.setDrawColor(1);
  u8g2.sendBuffer();    
}

void L1N1P1(){
  L1_position = 1;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 0, 125, 17);  
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 15,"信息输入");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"设置播报音量");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"设置播报语速");   
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 62,"返回上级菜单");     
  u8g2.sendBuffer();    
}

void L1N1P2(){
  L1_position = 2;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 17, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"信息输入");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 31,"设置播报音量");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"设置播报语速");   
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 62,"返回上级菜单");     
  u8g2.sendBuffer();    
}

void L1N1P3(){
  L1_position = 3;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 33, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"信息输入");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"设置播报音量");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 47,"设置播报语速");   
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 62,"返回上级菜单");     
  u8g2.sendBuffer();    
}

void L1N1P4(){   
  u8g2.clearBuffer();
  u8g2.drawBox(0, 49, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"信息输入");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"设置播报音量");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"设置播报语速");   
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 62,"返回上级菜单");   
  u8g2.setDrawColor(1);   
  u8g2.sendBuffer();    
}

void L1N2P1(){
  L1_position = 4;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 0, 125, 17);  
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 15,"当前使用的GPT模型");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"可选择GPT模型");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"发送回答到邮箱");   
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 62,"返回上级菜单");     
  u8g2.sendBuffer();    
}

void L1N2P2(){
  L1_position = 5;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 17, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"当前使用的GPT模型");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 31,"可选择GPT模型");
  u8g2.setDrawColor(1); 
  u8g2.drawUTF8(0, 47,"发送回答到邮箱");   
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 62,"返回上级菜单");     
  u8g2.sendBuffer();    
}

void L1N2P3(){
  L1_position = 6;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 33, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"当前使用的GPT模型");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"可选择GPT模型");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 47,"发送回答到邮箱");   
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 62,"返回上级菜单");    
  u8g2.sendBuffer();    
}

void L1N2P4(){ 
  u8g2.clearBuffer();
  u8g2.drawBox(0, 49, 125, 15);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"当前使用的GPT模型");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"可选择GPT模型");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"发送回答到邮箱");   
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 62,"返回上级菜单"); 
  u8g2.setDrawColor(1);     
  u8g2.sendBuffer();    
}

void L2N1P1(){
  L2_position = 1;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 0, 125, 17);  
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 15,"讯飞星火");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"阿里通义千问");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"miniMax大模型");
  u8g2.setDrawColor(1);  
  u8g2.drawUTF8(0, 62,"返回上级菜单"); 
  u8g2.setDrawColor(1);      
  u8g2.sendBuffer();    
}

void L2N1P2(){
  L2_position = 2;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 17, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"讯飞星火");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 31,"阿里通义千问");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"miniMax大模型");
  u8g2.setDrawColor(1);  
  u8g2.drawUTF8(0, 62,"返回上级菜单"); 
  u8g2.setDrawColor(1);      
  u8g2.sendBuffer();     
}

void L2N1P3(){
  L2_position = 3;
  u8g2.clearBuffer();
  u8g2.drawBox(0, 33, 125, 17);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"讯飞星火");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"阿里通义千问");
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(0, 47,"miniMax大模型");
  u8g2.setDrawColor(1);  
  u8g2.drawUTF8(0, 62,"返回上级菜单"); 
  u8g2.setDrawColor(1);      
  u8g2.sendBuffer();      
}

void L2N1P4(){
  u8g2.clearBuffer();
  u8g2.drawBox(0, 49, 125, 15);  
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 15,"讯飞星火");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 31,"阿里通义千问");
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 47,"miniMax大模型");
  u8g2.setDrawColor(0);  
  u8g2.drawUTF8(0, 62,"返回上级菜单"); 
  u8g2.setDrawColor(1);      
  u8g2.sendBuffer();    
}
void question_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"按下按键开始提问"); 
  u8g2.drawUTF8(0, 31,"再次按按键提交问题");
  u8g2.drawUTF8(0, 47,"提问时长不超30秒");
  u8g2.drawUTF8(0, 62,"上下拨键返回上级");
  u8g2.sendBuffer();   
}
void recording_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"开始提问"); 
  u8g2.drawUTF8(0, 31,"提问时长不超30秒");
  u8g2.sendBuffer();  
}
void sendquestion_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"提交提问中");   
  u8g2.sendBuffer();   
}
void speech_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"回答中");   
  u8g2.sendBuffer();    
}
void STT_connextion_failed_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"语音识别服务器连"); 
  u8g2.drawUTF8(0, 31,"接失败，检查语音"); 
  u8g2.drawUTF8(0, 47,"账户信息输入，确"); 
  u8g2.drawUTF8(0, 62,"认账户是否有效");             
  u8g2.sendBuffer();   
}
void get_answer_failed_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"大模型连接失败"); 
  u8g2.drawUTF8(0, 31,"检查模型信息输入"); 
  u8g2.drawUTF8(0, 47,"确认账户是否有效");             
  u8g2.sendBuffer();   
}  
void no_GPT_selected_page(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"未选择GPT模型"); 
  u8g2.drawUTF8(0, 31,"返回上级菜单");  
  u8g2.sendBuffer();   
}
void no_answer_page(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"未发现回答文本"); 
  u8g2.drawUTF8(0, 31,"按键返回上级菜单");  
  u8g2.sendBuffer();  
}
void TTS_page(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"语音合成中");    
  u8g2.sendBuffer();  
}
void get_TTS_failed_page(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"语音合成连接失败"); 
  u8g2.drawUTF8(0, 31,"检查模型信息输入"); 
  u8g2.drawUTF8(0, 47,"确认账户是否有效");             
  u8g2.sendBuffer();   
}  
void videoQRcode(){
  u8g2.clearBuffer(); 
  u8g2.drawXBMP(5,3,58,58,QRcode);
  u8g2.drawUTF8(68, 15,"手机拍照");
  u8g2.drawUTF8(68, 31,"识别照片");
  u8g2.drawUTF8(68, 47,"中二维码"); 
  u8g2.sendBuffer();
}
void wifi_page1(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"WIFI连接中");    
  u8g2.sendBuffer();  
}
void wifi_page2(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"WIFI连接成功");    
  u8g2.sendBuffer();  
}
void gettime_page1(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"正在获取网络时间");
  u8g2.sendBuffer();  
}
void gettime_page2(){
  u8g2.clearBuffer();   
  u8g2.drawUTF8(0, 15,"网络时间获取成功");
  u8g2.sendBuffer();  
}
void hear_nothing(){
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15,"问题未被识别"); 
  u8g2.drawUTF8(0, 31,"请靠近本设备提问");
  u8g2.sendBuffer();      
}