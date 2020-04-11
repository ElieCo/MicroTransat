

class CommManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(CommManager)
  ~CommManager(){}

  void init(){
    m_lora.init();

    m_index_log = 0;
  }

  void go(){
    // check received messages
    String msg_rcv = m_lora.receive();
    if (msg_rcv.length() <= 0) return;
    m_db->setData("Msg_received", msg_rcv);

    // send message
    bool send_log = msg_rcv.indexOf("log")>= 0;
    if (send_log) {
      if (m_index_log == 0){
        m_actual_line = getLine();
        m_index_log++;
        sendLog(m_actual_line.substring(0,50), false);
      }
      else {
        m_index_log = 0;
        send_log = false;
        sendLog(m_actual_line.substring(50), true);
      }
    }
  }

  private:

  String getLine(){
    std::map<string, string> data = m_db->getAllData();
    String line = "";
    for (std::map<string, string>::iterator it = data.begin(); it != data.end(); ++it){
      line += it->second.c_str();
      if (it != data.end()) {
        line += ";";
      }
    }
    return line;
  }

  void sendLog(String msg, boolean end_communication)
  {
    if (end_communication){
      msg = msg + '~';
    }
    
    m_lora.send(msg);
  }


  LoRa m_lora;
  int m_index_log;
  String m_actual_line;
};
