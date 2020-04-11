// Librairie Lora
#include <RH_RF95.h>
#define LORA_SERIAL Serial4
#define LORA_FREQ 434.0

class LoRa
{
  public:
  LoRa(){}
  ~LoRa(){
    delete m_lora;
  }

  void init(){
    m_lora = new RH_RF95<HardwareSerial>(LORA_SERIAL);

    if (!m_lora->init()) {
      print("initialisation Lora : failed");
    }
    else {
      print("initialization Lora : OK");
      m_lora->setFrequency(LORA_FREQ);
    }
  }

  void send(String msg){
    uint8_t data[2 * msg.length()];
    msg.getBytes(data, sizeof(data));
    m_lora->send(data, sizeof(data));
    m_lora->waitPacketSent();
  }

  String receive(){
    String msg = "";

    if(lora.available())  // waiting for a response
    {
      uint8_t buf[10];
      uint8_t len = sizeof(buf);
      if(lora.recv(buf, &len))
      {
        msg = String((char *)buf);
        print(msg);
      } else {
        print("recv failed");
      }
    }

    return msg;
  }

  private:
  RH_RF95<HardwareSerial> *m_lora;
};
