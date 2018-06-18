#include <SoftwareSerial.h>
#include <TimerOne.h>

//#define Testdoxa

#define TIME_OUT_CNT  100
#define MAX_TX_SIZE   58
#define TIMEOUT_RET   2

#define HEADER          0xC0
#define NODE_AddressH   0x05
#define NODE_AddressL   0x02
#define SPEED           0x18
#define NODE_Channel    0x17
#define OPTION          0xC7


#define GW_AddressH     0x5
#define GW_AddressL     0x1
#define GW_Channel      0x17

#define M0pin   7
#define M1pin   8
#define AUXpin  A0
#define SOFT_RX 10
#define SOFT_TX 11

SoftwareSerial LoraSerial(SOFT_RX, SOFT_TX);  // RX, TX

//Cam bien anh sang Pin
#define cambien  12//Input sensor
int Led = 13;//LEd

//Test do xa:
uint8_t SequenceNum_Packet = 0;
uint8_t SequenceNum_ACK = 0;
uint8_t ACK_Received = 0;

//Mode: Auto or Handy?
#define modeAUTO    0xFF
#define modeHANDY   0xAA
#define lightON     0x11
#define lightOFF    0x22
uint8_t Mode = modeAUTO;  //default

//=== AUX ===========================================
bool AUX_HL;
bool ReadAUX()
{
  int val = analogRead(AUXpin);

  if(val<50){
    AUX_HL = LOW;
  }else {
    AUX_HL = HIGH;
  }
  return AUX_HL;
}


//Wait AUX High
int WaitAUX_H()
{
  int ret_var = 0;
  uint8_t cnt = 0;

  while ((ReadAUX() == LOW) && (cnt++ < 100)){
    Serial.print(".");
    delay(100);
  }
  Serial.println(""); 

  if (cnt >= 100 ) {
    ret_var = TIMEOUT_RET;
    Serial.println("Timeout!");
  }
  
  return ret_var;
}


//The setup function is called once at startup of the sketch
void setup()
{
  
  //GPIO Init
    //Lora 
  pinMode(M0pin, OUTPUT);
  pinMode(M1pin, OUTPUT);
  pinMode(AUXpin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
    //Sensor
  //pinMode(Led,OUTPUT);//pinMode xuất tín hiệu đầu ra cho led
  pinMode(cambien,INPUT);//pinMode nhận tín hiệu đầu vào cho cảm biê


  //Serial Init
  LoraSerial.begin(9600);
  Serial.begin(9600);
  Serial.println("GPIO & Serial are ready");


  //Setup Lora E32 Module - Send command through UART
  digitalWrite(M0pin, HIGH);
  digitalWrite(M1pin, HIGH);
  Serial.println("Mode 3 - Sleep");
  delay(10);
  if(WaitAUX_H() == TIMEOUT_RET) {exit(0);} //if timeout
  

  //Write CMD for Lora configuration
  //Header | Address High | Address Low | SPEED | Channel | Option
  uint8_t CMD[6] = {HEADER, NODE_AddressH, NODE_AddressL, SPEED, NODE_Channel, OPTION};
  LoraSerial.write(CMD, 6);
  WaitAUX_H();
  delay(1200);
  Serial.println("Setting Configure has been sent!");
  

  //Read Set configure
  while(LoraSerial.available()){LoraSerial.read();} //Clean Uart Buffer
  uint8_t READCMD[3] = {0xC1, 0xC1, 0xC1};
  LoraSerial.write(READCMD, 3);
  WaitAUX_H();
  delay(50);
  Serial.println("Reading Configure has been sent!");
  uint8_t readbuf[6];
  uint8_t Readcount, idx, buff;
  
  Readcount = LoraSerial.available();
  Serial.println(Readcount);
  if (Readcount){
    Serial.println("Setting Configure is:  ");
    for(idx = 0; idx < Readcount; idx++){
      readbuf[idx] = LoraSerial.read();
      delay(10);
    }
    Serial.print("Shutdown Mode: ");Serial.println(0xFF & readbuf[0],HEX);
    Serial.print("Address High: ");Serial.println(0xFF & readbuf[1],HEX);
    Serial.print("Address Low: ");Serial.println(0xFF & readbuf[2],HEX);
    Serial.print("Speed: ");Serial.println(0xFF & readbuf[3],HEX);
    Serial.print("Channel: ");Serial.println(0xFF & readbuf[4],HEX);
    Serial.print("Option: ");Serial.println(0xFF & readbuf[5],HEX);
  }


  //Mode 0 Normal
  digitalWrite(M0pin,LOW);  //MODE 0 - NORMAL
  digitalWrite(M1pin,LOW);
  Serial.println("Mode 0 - NORMAL");
  delay(10);
  if(WaitAUX_H() == TIMEOUT_RET) {exit(0);} //if timeout

  //Timer Setting
  //pinMode(LED_BUILTIN,OUTPUT);
  #ifdef Testdoxa
  Timer1.initialize(5000000); //1s 1 time
  #else
  Timer1.initialize(10000000); //5s 1 time
  #endif
  Timer1.attachInterrupt(timerIsr);
  Serial.println("Timer1 started!");

  //Setting Done
  Serial.println();
  Serial.println("---------------SETTING DONE-------------\n");
}


void blinkLED()
{
  static bool LedStatus = LOW;
  
  digitalWrite(LED_BUILTIN, LedStatus);
  LedStatus = !LedStatus;
}


// The loop function is called in an endless loop
void loop(){
  //Always waiting for  Received Message
  uint8_t i;
  uint8_t data_buf[100], data_len;
  uint8_t value = digitalRead(cambien);
  if(Mode == modeAUTO){  //Mode auto
      digitalWrite(Led,value);//xuất giá trị ra đèn LED
      }
  data_len = LoraSerial.available();


  #ifdef Testdoxa
  if (data_len > 0){
    Serial.print("ReceiveMsg: ");  Serial.print(data_len);  Serial.println(" bytes.");
    for(i=0;i<data_len;i++)
      data_buf[i] = LoraSerial.read();

     //if some data??? Mode auto or handy?
     //...SomeCodehere
     ACK_Received++;
     Serial.print("ACK Received = ");Serial.println(ACK_Received);
     Serial.print("Sequence Number (ACK) = "); Serial.println(data_buf[0]);
     if (data_buf[0] == 100){ //Sequence_num ACK = 100;
         float PRR_ACK = float(ACK_Received) / float(data_buf[0]);
         Serial.println("-------------PRR-----------------");
         Serial.print("PRR = "); Serial.println(PRR_ACK);
         Serial.println("---------------------------------");
         while(1);
     }    
}


  #else //system
  if (data_len > 0){
    Serial.print("ReceiveMsg: ");  Serial.print(data_len);  Serial.println(" bytes.");
    for(i=0;i<data_len;i++){
      data_buf[i] = LoraSerial.read();
      Serial.println(0xFF & data_buf[i], HEX);}
    if (data_buf[0] == modeHANDY)
      Mode = modeHANDY;
    else if (data_buf[0] == modeAUTO) Mode = modeAUTO;
    if (data_buf[0] == lightON) digitalWrite(Led, 1);
    if (data_buf[0] == lightOFF) digitalWrite(Led, 0);
}
#endif
  }

void timerIsr(){
  //After 1s send sensor data
  #ifdef Testdoxa
  //ReadSensor(); ->value: output
  uint8_t value = digitalRead(cambien);
  //ReadLedState();
  uint8_t LEDstate = digitalRead(Led);
  //SendMsg();
  //Frame: NodeID|Value|LEDstate
  if(WaitAUX_H() == TIMEOUT_RET) {exit(0);} //if timeout
    SequenceNum_Packet++;
    uint8_t FrameSend[8] = {GW_AddressH, GW_AddressL, GW_Channel, NODE_AddressH, NODE_AddressL, value, LEDstate, SequenceNum_Packet};
    LoraSerial.write(FrameSend, 8);
    Serial.println("Data hasbeen sent!");
    delay(10);




  #else //system
  //ReadSensor(); ->value: output
  uint8_t value = digitalRead(cambien);
  if(Mode == modeAUTO){  //Mode auto
      digitalWrite(Led,value);//xuất giá trị ra đèn LED
      }
   //ReadLedState();
   uint8_t LEDstate = digitalRead(Led);
   //Frame: NodeID|Value|LEDstate
   if(WaitAUX_H() == TIMEOUT_RET) {exit(0);} //if timeout
   SequenceNum_Packet++;
   uint8_t FrameSend[8] = {GW_AddressH, GW_AddressL, GW_Channel, NODE_AddressH, NODE_AddressL, value, LEDstate, SequenceNum_Packet};
   LoraSerial.write(FrameSend, 8);
   Serial.println("Data hasbeen sent!");
   delay(10);
  
    #endif
    
}

