#define serialDebug Serial1

#define DEBUGLEVEL 1

#ifndef SIMU
#define SIMU false
#endif

void initSerialDebug(){
  serialDebug.begin(9600);
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

float toMS(float speed){
  return speed/1.94384;
}

float toKnots(float speed){
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

template <class T>
class Average{
  public:
  Average(int len){
    m_buffer = new T[len];
    m_len = len;
    m_index = 0;

    for (int i = 0; i < m_len; i++) m_buffer[i] = 0;
  }
  ~Average(){
    delete m_buffer;
  }

  T average(T value){
    m_buffer[m_index] = value;
    m_index = (m_index + 1) % m_len;
    
    T total = T();
    for(int i = 0; i < m_len; i++){
      total += m_buffer[i];
    }

    return total/m_len;
  }

  private:
  T *m_buffer;
  int m_len;
  int m_index;
  
};

