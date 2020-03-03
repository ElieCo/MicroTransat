#include <RH_RF95.h>
#ifdef __AVR__
#include <SoftwareSerial.h>
SoftwareSerial SSerial(10, 11); // RX, TX
#define COMSerial SSerial
#define ShowSerial Serial 

RH_RF95<SoftwareSerial> rf95(COMSerial);
#endif

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define COMSerial Serial1
#define ShowSerial SerialUSB 

RH_RF95<Uart> rf95(COMSerial);
#endif

#ifdef ARDUINO_ARCH_STM32F4
#define COMSerial Serial
#define ShowSerial SerialUSB 

RH_RF95<HardwareSerial> rf95(COMSerial);
#endif

int led = 13;

void setup() 
{
    ShowSerial.begin(9600);
    
    pinMode(led, OUTPUT); 
    
    if(!rf95.init())
    {
        ShowSerial.println("init failed");
        while(1);
    } 
    
    rf95.setFrequency(434.0);
}

void loop()
{
  String courrier = "";
  // lecture des données venants du port série
  if (Serial.available() > 0) {
    courrier = Serial.readString();
  }
  else {
    courrier = "42";
  }
  // send a specific order, or
  // send a witchever message to allow the boat to respond
  uint8_t data[2 * courrier.length()];
  courrier.getBytes(data, sizeof(data));

  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();
  
  if(rf95.waitAvailableTimeout(1000))
  { 
    digitalWrite(led, HIGH);
    uint8_t buf[400];
    uint8_t len = sizeof(buf);
    if(rf95.recv(buf, &len))
    { 
        digitalWrite(led, HIGH);
        ShowSerial.println((char*)buf);
    }
    else
    {
        ShowSerial.println("recv failed");
    }
      
    digitalWrite(led, LOW);
  }
}


