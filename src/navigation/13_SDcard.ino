#include <SD.h>

class SDcard
{
  public:

  SDcard(){}
  ~SDcard(){
    m_file.close();
  }

  void initLog(){
    
    if (!SD.begin(BUILTIN_SDCARD)) {
      Serial1.println("initialization carte SD : failed");
    }
    else {
      Serial1.println("initialization carte SD : OK");
    }
    m_file = SD.open("log.csv", FILE_WRITE);
  }

  void write(String line){
    m_file.println(line);
  }

  void stop(){
    m_file.close();
  }

  File m_file;
};

