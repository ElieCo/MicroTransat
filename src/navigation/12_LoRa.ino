// Librairie Lora
#include <RH_RF95.h>
#define LORA_SERIAL Serial4
#define LORA_FREQ 434.0
RH_RF95<HardwareSerial> global_lora(LORA_SERIAL);

class LoRa
{
  public:
  LoRa(){
    if (!m_lora.init()) {
      print("initialisation Lora : failed");
    }
    else {
      print("initialization Lora : OK");
      m_lora.setFrequency(LORA_FREQ);
    }
  }
  ~LoRa(){}

  void send(String msg){
    uint8_t data[2 * msg.length()];
    msg.getBytes(data, sizeof(data));
    m_lora.send(data, sizeof(data));
    m_lora.waitPacketSent();
  }

  String receive(){
    return "";
  }
  
  private:
  RH_RF95<HardwareSerial> m_lora = global_lora;
};

