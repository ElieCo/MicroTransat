

class Simulator{
  public:
  Simulator(DataBase* db)
    : m_db(db)
  {}
  ~Simulator(){}

  void SimulateGps(){
    
  }

  float getWindDirection(){
    return m_wind_direction;
  }
  void setWindDirection(float wind_dir){
    m_wind_direction = wind_dir;
    from0to360(m_wind_direction);
  }
  void moveWindDirection(float diff){
    m_wind_direction = m_wind_direction + diff;
    from0to360(m_wind_direction);
  }

  private:
  DataBase* m_db;

  float m_wind_direction;
  float m_actual_speed;
  float actual_cap;

  void calculateSpeed(float wing_angle){

    float lift_angle = 0.0;
    if (wing_angle < 0) lift_angle = m_wind_direction + (wing_angle + 90);
    else lift_angle = m_wind_direction - (wing_angle + 90);
    from0to360(lift_angle);

    float diff_angle = abs(actual_cap - lift_angle);
    float prop_coeff = cos(radians(diff_angle));
    
    float max_speed = 0.5;
    float theorical_speed = max_speed * prop_coeff;

    m_actual_speed = m_actual_speed + 0.3*(theorical_speed - m_actual_speed);
  }
};

