#define SERIAL_SIMU Serial

#define START 0xF1
#define STOP 0xF3

class SimuComm {
  public:
    SimuComm()
    : m_index(0)
    {
      memset(m_buffer, 0, sizeof(m_buffer));

      initSerial();
      m_inSimulation = simuCheck();
    }
    ~SimuComm(){}

    struct GpsData{
      double lat, lng;
      float altitude;
      bool fix;
      unsigned short fix_quality;
      unsigned short satellites;
      unsigned int fix_age, time, date;
      float speed, course;
      int hdop;
    };

    bool inSimulation(){
      return m_inSimulation;
    }

    void simuServo(int pin, float angle){
      m_simuData.servoData.set(pin, angle);
      sendData();
    }
    GpsData simuGps(){
      readData();
      return m_simuData.gpsData;
    }

  private:

    struct SimuData{
      Map<int, float> servoData;
      GpsData gpsData;
    };

    void initSerial(){
      SERIAL_SIMU.setTimeout(100);
      SERIAL_SIMU.begin(115200);
    }

    bool simuCheck(){
      int t0 = millis();
      while (millis() - t0 < 10000){
        sendData();

        delay(500);

        if (readData())
          return true;
      }
      return false;
    }

    void sendData(){
      int nb = m_simuData.servoData.size();
      uint8_t data[1 + nb*(4 + 4) + 1];

      data[0] = static_cast<uint8_t>(START);

      for (int i = 0; i < nb; i++){
        int pin = m_simuData.servoData.keyAt(i);
        float angle = m_simuData.servoData.valueAt(i);
        memcpy(&data[1 + i*8], &pin, 4);
        memcpy(&data[1 + i*8 + 4], &angle, 4);

      }

      data[sizeof(data)-1] = static_cast<uint8_t>(STOP);

      SERIAL_SIMU.write(data, sizeof(data));
    }

    bool readData(){
      uint8_t recv[51];
      unsigned int recv_size = SERIAL_SIMU.readBytes((char*)recv, sizeof(recv));
      for (unsigned int i = 0; i < recv_size; i++){
        m_buffer[m_index] = recv[i];
        m_index = (m_index+1) % sizeof(m_buffer);
      }

      return decrypteBuffer();
    }

    bool decrypteBuffer(){
      bool result = false;

      // copy the buffer in a other one to have it in the right order
      uint8_t buffer[sizeof(m_buffer)];
      for (unsigned int i = 0; i < sizeof(m_buffer); i++){
        buffer[i] = m_buffer[ (m_index + i) % sizeof(m_buffer) ];
      }

     // check if the last readed byte is a STOP
      if (buffer[sizeof(buffer)-1] == STOP){
        // check if 50 before STOP there is a START
        if (buffer[sizeof(buffer)-1 - 50] == START){
          result = true;
          uint8_t data[49];
          m_simuData.gpsData = lastDataToGpsData(buffer);
        }
        // clear the buffer
        memset(buffer, 0, sizeof(buffer));
      }

      // copy the tmp buffer into the real buffer with the right offset
      for (unsigned int i = 0; i < sizeof(buffer); i++){
        m_buffer[ (m_index + i) % sizeof(m_buffer) ] = buffer[i];
      }
      
      return result;
    }

    GpsData lastDataToGpsData(uint8_t *buffer){
      GpsData data;

      int offset = sizeof(m_buffer)-1 - 49;
      
      memcpy(&data.lat, &buffer[offset+0], 8);
      memcpy(&data.lng, &buffer[offset+8], 8);
      memcpy(&data.altitude, &buffer[offset+16], 4);
      memcpy(&data.fix, &buffer[offset+20], 1);
      memcpy(&data.fix_quality, &buffer[offset+21], 2);
      memcpy(&data.satellites, &buffer[offset+23], 2);
      memcpy(&data.fix_age, &buffer[offset+25], 4);
      memcpy(&data.time, &buffer[offset+29], 4);
      memcpy(&data.date, &buffer[offset+33], 4);
      memcpy(&data.speed, &buffer[offset+37], 4);
      memcpy(&data.course, &buffer[offset+41], 4);
      memcpy(&data.hdop, &buffer[offset+45], 4);

      return data;
    }

    bool m_inSimulation;
    SimuData m_simuData;
    uint8_t m_buffer[255];
    unsigned short m_index;
};

SimuComm simuComm = SimuComm();
