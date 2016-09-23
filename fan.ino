#include<AM2321.h>
#include <Wire.h>
#include<IRremote.h>
#include <U8glib.h>
#include <SoftwareSerial.h>

//#define my_Serial mySerial
#define my_Serial Serial1
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
//-------字体设置，大、中、小
#define setFont_L u8g.setFont(u8g_font_7x13)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
#define setFont_S u8g.setFont(u8g_font_fixed_v0r)

const int motor1=8;
double TTemperature=30;//读取温度
unsigned long motorOn=100,motorOff=100;//电机开关时间
unsigned long sensor_time;//读取温度时间
int motorStatus;//电机状态
double x=0;//转速参数

//PIR传感器
const int vibrationPin1=12;
const int vibrationPin2=A2;
const int vibrationPin3=A6;

int vibrationState1;
int vibrationState2;
int vibrationState3;

//摆头电机
#define motor2_pinB 5
#define motor2_pinA 7

int RECV_PIN=A0;//红外接口

IRrecv irrecv(RECV_PIN);

decode_results results;

boolean fan_model = true;//风扇模式
boolean fan_status =false;//风扇状态

void setup() {

  Serial.begin(115200);
  my_Serial.begin(9600);
  pinMode(motor1, OUTPUT);

  pinMode(vibrationPin1,INPUT);
  pinMode(vibrationPin2,INPUT);
  pinMode(vibrationPin3,INPUT);

  pinMode(motor2_pinB, OUTPUT);
  pinMode(motor2_pinA, OUTPUT);
  
  sensor_time=millis();
  irrecv.enableIRIn();//开启红外接收

}
char *ptr;
void loop() {
  while (my_Serial.available())
  {
    char c = my_Serial.read();
    delay(2);
    
    if (c == 'Z'){
    fan_model = true;
     Serial.println("intell");
    
   }
   if (c == 'N'){
    fan_model = false;
     Serial.println("un");
   }
    if (c == 'O'){
    fan_status = true;
    Serial.println("ON");
    x=50;
    Serial.println(x);
    }
    if (c == 'C'){
    fan_status = false;
    Serial.println("OFF");
    x=0;
    Serial.println(x);
    stop2();
    }
    if (c == 'R'){
    fullForward2();
    Serial.println("R");
    }
    if (c == 'L'){
    fullBackward2();
    Serial.println("L");
    }
    if (c == 'A'){
   
    Serial.println("ADD");
    if(x<200) x+=50;
    Serial.println(x);
    }
    if (c == 'D'){
    
    Serial.println("DOWN");
    if(x>0)
    x-=50;
    Serial.println(x);
    }
   
  }

  if (irrecv.decode(&results)) 
  {
    irrecv.resume();//接收下一状态
    
   if(results.value == 0x1FE48B7)
    {
      fan_model = !fan_model;//切换模式
    }
    
   else if ((results.value == 0xFFA857 || results.value == 0x1FE10EF || results.value == 0x1FE30CF)&&(fan_model == false))
    {
      fan_status = !fan_status;
      if(fan_status == true)//开启
      {
        Serial.println("ON");
        x=50;
        Serial.println(x);
      }
      else //停止
      {
        Serial.println("OFF");     
        x=0;
        Serial.println(x);
        stop2();
      }
    }
    else if ((results.value == 0xFF02FD || results.value == 0x1FEA05F || results.value == 0x1FEF807)&&(fan_model == false)&&(fan_status == true))
     {
      Serial.println("ADD");//加速
      if(x<200) x+=50;
      Serial.println(x);
     }
    else if ((results.value == 0xFF9867 || results.value == 0x1FED827 || results.value == 0x1FE708F)&&(fan_model == false)&&(fan_status == true))
      {
        Serial.println("DOWN");//减速
        if(x>0)
        x-=50;
        Serial.println(x);
      }
      else if((results.value == 0x1FE50AF)&&(fan_model == false)&&(fan_status == true))
      {
         Serial.println("TURN");
         fullForward2();
      }
    else
       results.value = 0;
   }
   
  if(fan_model == true)//智能模式
    {
      //温控
      unsigned long nowtime0=millis(); 
      if((nowtime0-sensor_time)>500)
      {
        TTemperature=Tread();
        x=(TTemperature-27)*50;//转速
        sensor_time=nowtime0;
      }
  //摆头
  int left=0;
  int leftmid=0;
  int right=0;
  int rightmid=0;
  int mid=0;
  int sall1=0;
  int sall2=0;
  int none=0;
  vibrationState1 = digitalRead(vibrationPin1);
  vibrationState2 = digitalRead(vibrationPin2);
  vibrationState3 = digitalRead(vibrationPin3);
  for(int count=0;count<100;count++){
  if(vibrationState1 == LOW&&vibrationState2 == LOW&&vibrationState3 == HIGH)
  {
     mid++;
   }
  else if(vibrationState1 == HIGH&&vibrationState2 == LOW&&vibrationState3 == HIGH)
  {
    leftmid++;
   }
  else if(vibrationState1==HIGH&&vibrationState2 == LOW&&vibrationState3 == LOW)
  {
    left++;
   }
  else if(vibrationState1 == LOW&&vibrationState2 == HIGH&&vibrationState3 == HIGH)
  {
    rightmid++;
   }
  else if(vibrationState1 == LOW&&vibrationState2 == HIGH&&vibrationState3 == LOW)
  {
    right++;
   }
  else if(vibrationState1 == HIGH&&vibrationState2 == HIGH&&vibrationState3 == LOW)
  {
    sall1++;
   }
  else if(vibrationState1 == HIGH&&vibrationState2 == HIGH&&vibrationState3 == HIGH)
  {
    sall2++;
   }
   else
   {
    none++;
   }
  }
  int MAX=max(max(max(left,right),max(leftmid,rightmid)),max(max(sall1,sall2),max(mid,none)));
  if(left==MAX){
    Serial.println("100");
    fullForward2();
    delay(1000);
    vacillate();
  }
  else if(leftmid==MAX){
    Serial.println("101");
    fullForward2();
    delay(1000);
    vacillate();
  }
  else if(mid==MAX){
    Serial.println("001");
    vacillate();
  }
  else if(rightmid==MAX){
    Serial.println("011");
    fullBackward2();
    delay(1000);
    vacillate();
  }
  else if(right==MAX){
    Serial.println("010");
    fullBackward2();
    delay(1000);
    vacillate();
  }
  else if((sall1==MAX)||(sall2==MAX)){
    Serial.println("111");
    fullForward2();
  }
  else{
    Serial.println("OFF");
   stop2();
  } 
    }
     /*  else //非智能模式
      {
        Serial.println("non intelligent");
      }*/
      speedchange(x);
//数显 
u8g.firstPage();  
do {
    temp_print(TTemperature);
  } while( u8g.nextPage() );
}
//温度读取
double Tread()
{
  double t;
  AM2321 am2321;
  am2321.read();
  t=am2321.temperature/10.0;
  Serial.println(t);
  return t;
}

//转速控制x=0-200
void speedchange(double x)
{
  unsigned long nowtime=millis();
  if(x>200) x=200;
  if(x<5) x=0;
    if(motorStatus==HIGH)            
    {
      if(nowtime>motorOn)          
      {
        motorOn=nowtime;             
        motorOff=nowtime+200-x;     
        digitalWrite(motor1,LOW);      
        motorStatus=LOW;                
      }
    }

if(motorStatus==LOW)
{      
  if(nowtime>motorOff)
  {   
    motorOff=nowtime;
    motorOn=nowtime+x;
    digitalWrite(motor1,HIGH);
    motorStatus=HIGH;
  }
}
}

//摇头左右
void fullForward2() {
  //digitalWrite(motor2_pinB, HIGH);
  analogWrite(motor2_pinB,255);
  digitalWrite(motor2_pinA, LOW);
}
void fullBackward2() {
  analogWrite(motor2_pinA,255);
  digitalWrite(motor2_pinB, LOW);
}

//小幅旋转
void vacillate()
{
  fullForward2();
  delay(500);
  fullBackward2();
  delay(500);
}
//摆头停
void stop2(){
  digitalWrite(motor2_pinB, LOW);
  digitalWrite(motor2_pinA, LOW);
}


//风停
void stop1() {
  digitalWrite(motor1, LOW);
}
//温度显示LED
void temp_print(double i)
{
   u8g.setFont(u8g_font_7x13);
   u8g.setPrintPos(5, 40);
   u8g.print("temperature:");
   u8g.print(i);
}
