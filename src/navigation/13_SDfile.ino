#include <SD.h>

class SDfile
{
  public:

  SDfile(){
    m_file_opened = false;
  }
  ~SDfile(){
    m_file.close();
  }

  void init(String filename){

    if (SD.begin(BUILTIN_SDCARD)) {
      Serial1.println("initialization carte SD : OK");
      m_file = SD.open(filename.c_str(), FILE_WRITE);
      m_file_opened = true;
    }
    else {
      Serial1.println("initialization carte SD : failed");
      m_file_opened = false;
    }
  }

  void write(String line){
    if (!m_file_opened) return;

    m_file.println(line);
    m_file.flush();
  }

  void close(){
    m_file.close();
  }

  File m_file;
  bool m_file_opened;
};
