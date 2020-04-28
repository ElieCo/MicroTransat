// Librairie Lora
#include <RH_RF95.h>
#define LORA_SERIAL Serial4
#define LORA_FREQ 434.0

#define START_HEADER 0xF1
#define HEADER_LEN 2
#define MAX_SENDING_SIZE 100
#define MAX_EXCHANGE_TIME 5000

class LoRa
{
  public:
  LoRa()
    : m_state(WAITING)
    , m_sending_msg("")
    , m_send_buffer("")
    , m_recv_buffer("")
    , m_last_msg_received("")
    , m_new_msg(false)
    , m_have_to_send(false)
    , m_timer(0)
  {}
  ~LoRa(){
    delete m_lora;
  }

  void init(){
    m_lora = new RH_RF95<HardwareSerial>(LORA_SERIAL);

    if (m_lora->init()) {
      print("initialization Lora : OK");
      m_lora->setFrequency(LORA_FREQ);
    }
    else {
      print("initialisation Lora : failed");
    }
  }

  bool update(){
    switch(m_state){
      case WAITING:
        processWait();
        break;
      case SENDING:
        processSend();
        break;
      case RECEIVING:
        processReceive();
        break;
    }

    // check timeout
    if (m_state != WAITING && m_timer + MAX_EXCHANGE_TIME < millis()){

      if (m_state == SENDING){
        // reinit m_sender
        m_sending_msg = "";
        m_send_buffer = "";
      }

      if (m_state == RECEIVING){
        // reinit receiver
        m_recv_buffer = "";
      }

      m_state = WAITING;
    }

    bool result = m_new_msg;
    m_new_msg = false;
    return result;
  }

  void sendMessage(String msg){
    if(m_sending_msg == "")
      m_sending_msg = msg;
      m_have_to_send = true;
  }

  String getLastMessage(){
    return m_last_msg_received;
  }

  private:
  RH_RF95<HardwareSerial> *m_lora;

  enum COMM_STATE { WAITING = 0, SENDING = 1, RECEIVING = 2 };

  COMM_STATE m_state;

  String m_sending_msg;
  String m_send_buffer;
  String m_recv_buffer;
  String m_last_msg_received;
  bool m_new_msg;
  bool m_have_to_send;

  unsigned long m_timer;

  void processWait(){
    if (checkRecvMsg())
      m_state = RECEIVING;

    else if (m_sending_msg != "")
      m_state = SENDING;

    // start timer
    if (m_state != WAITING)
      m_timer = millis();
  }

  void processSend(){

    if (m_have_to_send){
      // If the buffer is empty, it's the first message.
      bool is_first_msg = m_send_buffer == "";

      // If it's the first message, put it in the buffer.
      if (is_first_msg)
        m_send_buffer = m_sending_msg;

      // Get the part of the message we have to send and update the buffer.
      String part_to_send = m_send_buffer.substring(0, MAX_SENDING_SIZE-HEADER_LEN);
      m_send_buffer = m_send_buffer.substring(MAX_SENDING_SIZE-HEADER_LEN);

      // Now if the buffer it empty it's the last message we have to send.
      bool is_last_msg = m_send_buffer == "";

      // Get the tag depending of it's the first and/or last message.
      char tag = (is_first_msg ? 0xF0 : 0) + (is_first_msg ? 0x0F : 0);

      // Create a message with the header and the data.
      String msg_to_send = String(char(START_HEADER)) + String(tag) + part_to_send;
      // Send it.
      send(msg_to_send);
      // Remember we are waiting for the acknowledge.
      m_have_to_send = false;

      // If it was the last message, erase the message to send.
      if (is_last_msg) m_sending_msg = "";

    } else {
      // Check if we have received an acknowledge.
      checkRecvMsg();
    }

    // Transition
    if (m_sending_msg == ""){
      m_state = WAITING;
    }
  }

  void processReceive(){
    // Check if we have a new message and treat it.
    checkRecvMsg();

    // Transition
    if (m_recv_buffer == ""){
      // Just say that there is a new message.
      m_new_msg = true;
      m_state = WAITING;
    }
  }

  bool checkRecvMsg(){
    String msg = "";

    uint8_t buf[1];
    uint8_t len = sizeof(buf);

    // Read byte by byte the new message.
    while(m_lora && m_lora->recv(buf, &len))
      msg += String((char *)buf);

    if (msg != ""){
      treatMsg(msg);
      return true;
    } else {
      return false;
    }

  }

  void treatMsg(String msg){
    // Put the msg in a buffer.
    int len = msg.length();
    uint8_t buf[len];
    msg.getBytes(buf, len);

    // If the length of the message is at least equals to the header length
    //  and if the message start with the START_HEADER.
    if(len >= HEADER_LEN && buf[0] == START_HEADER){
      // Check if it's the first and/or last message.
      bool is_first_msg = buf[1] & 0xF0;
      bool is_last_msg = buf[1] & 0x0F;

      // If it's not the first message but we have nothing in the buffer we missed the first message
      //  so we quit now.
      if (!is_first_msg && m_recv_buffer != "")
        return;

      // If there is data after the header append it in the buffer.
      if (len > HEADER_LEN)
        m_recv_buffer += msg.substring(HEADER_LEN+1);

      // If it's an acknowledge (len = HEADER_LEN && is_first_msg && is_last_msg) remember
      //  we have to send the next and quit now
      else if (is_first_msg && is_last_msg){
        m_have_to_send = true;
        return;
      }

      // If it was the last message copy the buffer in the message and erase the buffer.
      if (is_last_msg){
        m_last_msg_received = m_recv_buffer;
        m_recv_buffer = "";
      }
    }
  }

  void send(String msg){
    uint8_t data[msg.length()];
    msg.getBytes(data, sizeof(data));
    if(m_lora) {
      m_lora->send(data, sizeof(data));
      m_lora->waitPacketSent();
    }
  }
};
