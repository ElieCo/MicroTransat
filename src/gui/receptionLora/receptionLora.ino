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
    ShowSerial.begin(115200);
    ShowSerial.println("RF95 server test.");
    
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
  if(rf95.available())
  {  
    uint8_t buf[400];
    uint8_t len = sizeof(buf);
    if(rf95.recv(buf, &len))
    {
        digitalWrite(led, HIGH);
        ShowSerial.println((char*)buf);

        /*
        // Send a reply
        uint8_t data[] = "And hello back to you";
        rf95.send(data, sizeof(data));
        rf95.waitPacketSent();
        ShowSerial.println("Sent a reply");
        */
    }
    else
    {
        ShowSerial.println("recv failed");
    }
  }
}


