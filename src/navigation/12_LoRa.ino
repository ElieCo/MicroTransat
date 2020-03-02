// Librairie Lora
#include <RH_RF95.h>
#define LORA_SERIAL Serial4
#define LORA_FREQ 434.0

class LoRa
{
  public:
  LoRa(){
    m_lora = new RH_RF95<HardwareSerial>(LORA_SERIAL);
    
    if (!m_lora->init()) {
      print("initialisation Lora : failed");
    }
    else {
      print("initialization Lora : OK");
      m_lora->setFrequency(LORA_FREQ);
    }
  }
  ~LoRa(){
    delete m_lora;
  }

  void send(String msg){
    uint8_t data[2 * msg.length()];
    msg.getBytes(data, sizeof(data));
    m_lora->send(data, sizeof(data));
    m_lora->waitPacketSent();
  }

  String receive(){
    return "";
  }
  
  private:
  RH_RF95<HardwareSerial> *m_lora;
};

