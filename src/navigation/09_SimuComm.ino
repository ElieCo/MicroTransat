#define SERIAL_SIMU Serial1

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
    }
    GpsData simuGps(){
      updateData();
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
      sendData();
      int t0 = millis();
      while (millis() - t0 < 10000){

        delay(500);

        if (readData())
          return true;
      }
      return false;
    }

    bool updateData(){
      bool result = readData();
      sendData();
      return result;
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
      unsigned int recv_size = SERIAL_SIMU.readBytes(recv, sizeof(recv));
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
        buffer[i] = m_buffer[ (m_index+i) % sizeof(m_buffer) ];
      }

      // look for the START
      unsigned int index = 0;
      bool isStart = false;
      for (; index < sizeof(buffer); index++){
        if (buffer[index] == START){
          isStart = true;
          break;
        } else {
          // write zeros in the buffer while there is no start
          m_buffer[index] = 0;
        }
      }

      // return if there is no START
      if (!isStart) return result;

      // check if there is also a STOP
      if (index + 51 >= sizeof(buffer)){
        if (buffer[index + 50] == STOP){
          // read data and write zeros (make sure we don't write twice the same message)
          memcpy(&m_simuData.gpsData, &buffer[index+1], 49);
          memset(&buffer[index], 0, 51);
          result = true;

        } else if (buffer[index + 50] != 0){
          // if there is something but is not the stop,
          // it have to be that we didn't get the end of this message, so we erase this start
          m_buffer[index] = 0;
        }
      }

      // copy the tmp buffer into the real buffer with the right offset
      for (unsigned int i = 0; i < sizeof(buffer); i++){
        m_buffer[ (m_index+i) % sizeof(m_buffer) ] = buffer[i];
      }

      return result;
    }

    bool m_inSimulation;
    SimuData m_simuData;
    uint8_t m_buffer[255];
    unsigned short m_index;
};

SimuComm simuComm = SimuComm();
