#include <SD.h>

class SDcard
{
  public:

  SDcard(){
    m_file_opened = false;
  }
  ~SDcard(){
    m_file.close();
  }

  void initLog(){

    if (SD.begin(BUILTIN_SDCARD)) {
      Serial1.println("initialization carte SD : OK");
      m_file = SD.open("log.csv", FILE_WRITE);
      m_file_opened = true;
    }
    else {
      Serial1.println("initialization carte SD : failed");
      m_file_opened = false;
    }
  }

  void log(String line){
    if (!m_file_opened) return;

    m_file.println(line);
    m_file.flush();
  }

  void stop(){
    m_file.close();
  }

  File m_file;
  bool m_file_opened;
};
