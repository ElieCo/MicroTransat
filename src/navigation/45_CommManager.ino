

class CommManager : public BaseManager
{
  public:
  BASIC_CONSTRUCTOR(CommManager)
  ~CommManager(){}

  void init(){
    db_msg_received.init(m_db, "Msg_received", String(""));

    m_lora.init();

    m_index_log = 0;
  }

  void go(){

    // check received messages
    db_msg_received.set(m_lora.receive());
    // Return if there is no message.
    if (db_msg_received.get().length() <= 0) return;

    // send message
    bool send_log = db_msg_received.get().indexOf("log")>= 0;
    if (send_log) {
      if (m_index_log == 0){
        // Send the first part of the message.
        m_actual_line = getLine();
        m_index_log++;
        sendLog(m_actual_line.substring(0,50), false);
      }
      else {
        // Send the second pat of the message
        if (m_actual_line.length() <= 50*m_index_log +50){
          sendLog(m_actual_line.substring(m_index_log*50)+"~", true);
          m_index_log = 0;
          send_log = false;
        }
        else {
          sendLog(m_actual_line.substring(m_index_log*50,m_index_log*50+50), false);
          m_index_log++;
        }
        print("partie ", m_index_log);
      }
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

  void sendLog(String msg, boolean end_communication)
  {
    if (end_communication){
      msg = msg + '~';
    }

    m_lora.send(msg);
  }


  LoRa m_lora;
  unsigned int m_index_log;
  String m_actual_line;
};
