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

  bool init(String filename, bool sd_initialized, bool read = true){

    if (SD.begin(BUILTIN_SDCARD) || sd_initialized) {
      if (!sd_initialized) print("initialization carte SD : OK");
      if (read) m_file = SD.open(filename.c_str(), FILE_READ);
      else m_file = SD.open(filename.c_str(), FILE_WRITE);
      if (m_file) {
        m_file_opened = true;
      } else {
        m_file_opened = false;
        print("Error: Open file failed", filename);
      }
      return true;
    }
    else {
      print("initialization carte SD : failed");
      m_file_opened = false;
      return false;
    }
  }

  void write(String line){
    if (!m_file_opened) return;

    m_file.println(line);
    m_file.flush();
  }

  String readAll(){
    String result = "";
    if (m_file_opened) {
      char c;
      while(m_file.available()) {
        c = m_file.read();
        result += c;
      }
    }
    return result;
  }

  void close(){
    m_file.close();
  }

  File m_file;
  bool m_file_opened;
};
