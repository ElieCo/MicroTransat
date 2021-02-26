#include <SD.h>
#include <SPI.h>

const int chipSelect = BUILTIN_SDCARD;

#define TERMINAL_MODE false

class SerialManager{
 public:

  SerialManager(int baudrate = 115200, int timeout = 1){
    m_baudrate = baudrate;
    m_timeout = timeout;
    m_buffer = "";
  }

  bool open(){
    Serial.setTimeout(m_timeout);
    Serial.begin(m_baudrate);
    Serial.println("");
    return (Serial == true);
  }

  void write(String data){
    Serial.print(data);
  }

  void print(String data){
    Serial.println(data);
  }

  String readLine(unsigned int timeout = 0){
    return readUntil("\r", timeout);

  }

  String readUntil(String end_pattern,unsigned int timeout = 10000){
    int t0 = millis();

    do{
      if (Serial.available() > 0){
        String d = Serial.readString();
        m_buffer += d;
        if (TERMINAL_MODE){
          d.replace("\r", "\r\n");
          Serial.print(d);
        }
      }
    } while (m_buffer.indexOf(end_pattern) < 0 && millis() - t0 < timeout);

    int index = m_buffer.indexOf(end_pattern);
    if (index >= 0){
      String line = m_buffer.substring(0, index); // without end pattern
      m_buffer = m_buffer.substring(index + end_pattern.length());
      return line;

    } else {
      return "";
    }

  }

 private:
  String m_buffer;
  int m_baudrate;
  int m_timeout;

};

class FilesManager{
public:
  FileManager(){
    m_serial = SerialManager(115200);
  }

  void init(){
    m_serial.open();
    Serial.print("Initializing SD card...");

    if (!SD.begin(chipSelect)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");
  }

  void go(){
    String line = m_serial.readLine();

    if (line.length() == 0)
      return;
      
    if (TERMINAL_MODE)
        m_serial.print("");

    String line_without_space = line;
    line_without_space.replace(" ", "");

    if (line_without_space.equals("ls"))
      ls();
      
    else if (line_without_space.equals("ping"))
      ping();
      
    else if (line.indexOf("cat ") >= 0)
      cat(line);
      
    else if (line.indexOf("rm ") >= 0)
      rm(line);
      
    else if (line.indexOf("touch ") >= 0)
      touch(line);
      
    else if (line.indexOf("create ") >= 0)
      create(line);
      
    else
      if (TERMINAL_MODE)
        m_serial.print("Error: Unknown command " + line);
       
    if (TERMINAL_MODE)
        m_serial.print("");
  }

  void ls(){
    String output;
    listDirectoryFiles(SD.open("/"), output);
    m_serial.print(output);
  }

  void ping(){
    m_serial.print("pong");
  }

  void cat(String line){
    line.replace("cat ", "");
    line.replace(" ", "");
    String filename = line;

    String output;
    File file = SD.open(filename.c_str());
    while (file.available() > 0) {
      output += file.readString();
    }
    file.close();

    m_serial.print(output + "<<END OF FILE>>");
  }

  void touch(String line){
    line.replace("touch ", "");
    line.replace(" ", "");
    String filename = line;
    
    File file = SD.open(filename.c_str(), FILE_WRITE);
    file.close();
  }

  void rm(String line){
    line.replace("rm ", "");
    line.replace(" ", "");
    String filename = line;
    
    if (SD.remove(filename.c_str())){
      if (TERMINAL_MODE)
        m_serial.print("File deleted");
    }
    else {
      if (TERMINAL_MODE)
        m_serial.print("Error during deleting of " + filename);
    }
  }

  void create(String line){
    line.replace("create ", "");
    line.replace(" ", "");
    String filename = line;
    
    File file = SD.open(filename.c_str(), FILE_WRITE);

    // read until end_pattern
    String end_pattern = "<<END OF FILE>>";
    String input = m_serial.readUntil(end_pattern, 30000);

    file.print(input);
    
    file.close();
  }

private:

  void listDirectoryFiles(File directory, String &output, int indent = 0) {

    /* Ouvre le premier fichier */
    File entry = directory.openNextFile();
    while (entry) {

      /* Affiche l'indentation */
      for (byte i = 0; i < indent; i++) {
        output += ' ';
      }

      /* Affiche le nom du fichier courant */
      output += entry.name();

      /* Affichage récursif pour les dossiers */
      if (entry.isDirectory()) {
        output += "/\n\r";
        listDirectoryFiles(entry, output, indent + 1);

      } else {

        /* Affiche la taille pour les fichiers */
        output += "\n\r";
      }

      /* Ferme le fichier et ouvre le suivant */
      entry.close();
      entry = directory.openNextFile();
    }

    /* Revient au début du dossier */
    directory.rewindDirectory();
  }

  SerialManager m_serial;
};

FilesManager file_manager;

void setup(){
  file_manager = FilesManager();
  file_manager.init();
}

void loop(){
  file_manager.go();
}

