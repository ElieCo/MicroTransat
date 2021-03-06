#define serialDebug Serial

#define DEBUGLEVEL 1
#define LORA_IN_SIMU 0

void initSerialDebug(){
  serialDebug.begin(115200);
}

void print(String s){
  serialDebug.println(s);
}

void print(String s1, String s2){
  serialDebug.print(s1);
  serialDebug.print(" ");
  serialDebug.println(s2);
}

void print(String s1, String s2, String s3){
  serialDebug.print(s1);
  serialDebug.print(" ");
  serialDebug.print(s2);
  serialDebug.print(" ");
  serialDebug.println(s3);
}

float knotsToMs(float speed){
  return speed/1.94384;
}

float msToKnots(float speed){
  return speed*1.94384;
}

/**
 * Move the angle in a interval of 360 deg.
 * @param angle a reference to the angle.
 * @param ma the superior limit.
 */
void limit_angle(float &angle, float ma){
  if (DEBUGLEVEL >= 2) print("limit_angle", angle, ma);
  float mi = ma - 360;
  while(angle > ma) angle -= 360;
  while(angle < mi) angle += 360;
}

/**
 * Move the angle between 0 and 360 deg.
 * @param angle a reference to the angle.
 */
void from0to360(float &angle){
  limit_angle(angle, 360.0);
}

/**
 * Move the angle between -180 and 180 deg.
 * @param angle a reference to the angle.
 */
void from180to180(float &angle){
  limit_angle(angle, 180.0);
}

/**
 * Move the angle between 0 and 360 deg.
 * @param angle a pointer to the angle.
 */
void from0to360(float *angle){
  float a = *angle;
  limit_angle(a, 360.0);
  *angle = a;
}

/**
 * Move the angle between -180 and 180 deg.
 * @param angle a pointer to the angle.
 */
void from180to180(float *angle){
  float a = *angle;
  limit_angle(a, 180.0);
  *angle = a;
}

template <class T>
class Average{
  public:
  Average(){
    m_len = 0;
  }
  Average(int len){
    init(len);
  }
  ~Average(){
    delete m_buffer;
  }

  T average(T value){
    if (m_len == 0) return value;
    m_buffer[m_index] = value;
    m_index = (m_index + 1) % m_len;

    T total = T();
    for(int i = 0; i < m_len; i++){
      total += m_buffer[i];
    }

    return total/m_len;
  }

  void init(int len){
    m_buffer = new T[len];
    m_len = len;
    m_index = 0;

    clear();
  }

  void clear(){
    for (int i = 0; i < m_len; i++) m_buffer[i] = 0;
  }

  bool isFull(){
    return (m_buffer[m_index] != 0);
  }

  int size(){
    return m_len;
  }

  T at(int index){
    if (index < m_len && index >= 0)
      return m_buffer[index];
    else
      return T();
  }

  private:
  T *m_buffer;
  int m_len;
  int m_index;

};

class ObjectForDB{
 public:
  virtual String toString(){ return ""; };
};
