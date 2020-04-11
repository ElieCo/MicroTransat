

class CommManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(CommManager)
  ~CommManager(){}

  void go(){
    // check received messages
    String msg_rcv = m_lora.receive();
    m_db->setData("Msg_received", msg_rcv);

    // send message
    std::map<string, string> data = m_db->getAllData();
    for (std::map<string, string>::iterator it = data.begin(); it != data.end(); ++it){
      String var_name = it->first.c_str();
      String var_value = it->second.c_str();
      char separ = 0x1F;
      String msg = var_name + separ + var_value;

      m_lora.send(msg);
    }
  }

  LoRa m_lora;
};
