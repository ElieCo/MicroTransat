

class CommManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(CommManager)
  ~CommManager(){}

  void init(){
    db_msg_received.init(m_db, "Msg_received", false);

    m_lora.init();
  }

  void go(){
    // check received messages
    db_msg_received.set(m_lora.update());
    // Return if there is no message.
    if (!db_msg_received.get()) return;

    // send message
    bool send_log = m_lora.getLastMessage().indexOf("log")>= 0;
    if (send_log) {
      m_lora.sendMessage(getLine());
    }
  }

  void stop(){}

  private:

  DBData<String> db_msg_received;

  /**
   * Get a string line with all data which are in th DB.
   * @return line the string with all data in it.
   */
  String getLine(){
    Map<String, String> data = m_db->getAllData(true);
    String line = "";
    for (int i = 0; i < data.size(); i++){
      line += data.valueAt(i).c_str();
      if (i != data.size()-1) {
        line += ";";
      }
    }
    return line;
  }

  LoRa m_lora;
};
