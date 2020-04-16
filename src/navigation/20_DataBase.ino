
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
        void initData(String name, type data, int select = false) {   \
          if (!VAR_NAME(type).contains(name))                         \
            VAR_NAME(type).set(name, data);                           \
            selected.set(name, select);                               \
        }

#define MAP(type) MAP_TYPE(type) VAR_NAME(type);

#define ADD_TO_MAP_AS_STRING(map_name, type, only_selected)                                         \
        for (int i = 0; i < VAR_NAME(type).size(); i++){                                            \
          if (!only_selected || selected.get(VAR_NAME(type).keyAt(i)))                              \
            map_name.set(VAR_NAME(type).keyAt(i), String(VAR_NAME(type).valueAt(i)));               \
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
        ADD_TO_MAP_AS_STRING(map_string, double, only_selected)
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
