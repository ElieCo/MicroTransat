
#define VAR_NAME(type) DB##type

#define MAP_TYPE(type) Map<String, type>

#define GET_SET_AND_INIT(type)                                        \
        bool getData(String name, type &data) {                       \
          if (VAR_NAME(type).contains(name)) {                        \
            data = VAR_NAME(type).get(name);                          \
            return true;                                              \
          } else {                                                    \
            print("Error: try to get non valid data:", name.c_str()); \
            return false;                                             \
          }                                                           \
        }                                                             \
        void setData(String name, type data) {                        \
          if (!VAR_NAME(type).contains(name)) {                       \
            print("Variable not initialized:", name.c_str());         \
            return;                                                   \
          }                                                           \
          VAR_NAME(type).set(name, data);                             \
        }                                                             \
        type* initData(String name, type data, int select = false) {  \
          if (!VAR_NAME(type).contains(name)) {                       \
            VAR_NAME(type).set(name, data);                           \
            selected.set(name, select);                               \
          } else selected.set(name, (select || selected.get(name)));  \
          return VAR_NAME(type).getPtr(name);                         \
        }

#define MAP(type) MAP_TYPE(type) VAR_NAME(type);

#define ADD_TO_MAP_AS_STRING(map_name, type, only_selected)                                         \
        for (int i = 0; i < VAR_NAME(type).size(); i++){                                            \
          if (!only_selected || selected.get(VAR_NAME(type).keyAt(i)))                              \
            map_name.set(VAR_NAME(type).keyAt(i), String(VAR_NAME(type).valueAt(i)));               \
        }

#define ADD_TO_MAP_AS_STRING_WITH_DECIMAL(map_name, type, only_selected, decimals)                  \
        for (int i = 0; i < VAR_NAME(type).size(); i++){                                            \
          if (!only_selected || selected.get(VAR_NAME(type).keyAt(i)))                              \
            map_name.set(VAR_NAME(type).keyAt(i), String(VAR_NAME(type).valueAt(i), decimals));     \
        }

class DataBase {

  public:

    GET_SET_AND_INIT(String)
    GET_SET_AND_INIT(int)
    GET_SET_AND_INIT(unsigned)
    GET_SET_AND_INIT(long)
    GET_SET_AND_INIT(float)
    GET_SET_AND_INIT(double)
    GET_SET_AND_INIT(bool)

    Map<String, String> getAllData(bool only_selected = false){
        Map<String, String> map_string;

        ADD_TO_MAP_AS_STRING(map_string, int, only_selected)
        ADD_TO_MAP_AS_STRING(map_string, unsigned, only_selected)
        ADD_TO_MAP_AS_STRING(map_string, long, only_selected)
        ADD_TO_MAP_AS_STRING(map_string, float, only_selected)
        ADD_TO_MAP_AS_STRING_WITH_DECIMAL(map_string, double, only_selected, 7)
        ADD_TO_MAP_AS_STRING(map_string, bool, only_selected)
        ADD_TO_MAP_AS_STRING(map_string, String, only_selected)

        return map_string;
    }

  private:

    MAP(String)
    MAP(int)
    MAP(unsigned)
    MAP(long)
    MAP(float)
    MAP(double)
    MAP(bool)

    Map<String, int> selected;
};

template <class T>
class DBData {
  public:
  DBData(){
    data_ptr = NULL;
  }
  ~DBData(){}

    void init(DataBase* db, String name, T value, bool selected = false){
      data_ptr = db->initData(name, value, selected);
      m_prev_value = *data_ptr;
    }

  T get(){
    if (data_ptr) {
      m_prev_value = *data_ptr;
      return *data_ptr;
    }
    else return T();
  }

  void set(T value){
    if (data_ptr) {
      m_prev_value = *data_ptr;
      *data_ptr = value;
    }
  }

  void add(T value){
    if (data_ptr) {
      m_prev_value = *data_ptr;
      *data_ptr += value;
    }
  }

  bool hasChanged(){
    bool hasChanged = m_prev_value != *data_ptr;
    m_prev_value = *data_ptr;
    return hasChanged;
  }

  T* data_ptr;

  private:
  T m_prev_value;
};
