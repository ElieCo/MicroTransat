#include <map>
#include <string>
using namespace std;

#define VAR_NAME(type) DB##type

#define MAP_TYPE(type) std::map<std::string, type>

#define GET_SET_AND_INIT(type)                                        \
        bool getData(string name, type &data) {                       \
          if (VAR_NAME(type).count(name)) {                           \
            data = VAR_NAME(type)[name];                              \
            return true;                                              \
          } else {                                                    \
            print("Error: try to get non valid data:", name.c_str()); \
            return false;                                             \
          }                                                           \
        }                                                             \
        void setData(string name, type data) {                        \
          if (!VAR_NAME(type).count(name)) {                          \
            print("Variable not initialized:", name.c_str());         \
            return;                                                   \
          }                                                           \
          VAR_NAME(type)[name] = data;                                \
        }                                                             \
        void initData(string name, type data) {                       \
          VAR_NAME(type)[name] = data;                                \
        }

#define MAP(type) MAP_TYPE(type) VAR_NAME(type);

#define ADD_TO_MAP_AS_STRING(map_name, type)                                                            \
        for (MAP_TYPE(type)::iterator it = VAR_NAME(type).begin(); it != VAR_NAME(type).end(); ++it){   \
                map_name[it->first] = String(it->second).c_str();                                       \
        }

#define ADD_TO_MAP(map_name, type)                                                                      \
        for (MAP_TYPE(type)::iterator it = VAR_NAME(type).begin(); it != VAR_NAME(type).end(); ++it){   \
                map_name[it->first] = it->second;                                                       \
        }

class DataBase {

  public:

    GET_SET_AND_INIT(String)
    GET_SET_AND_INIT(string)
    GET_SET_AND_INIT(int)
    GET_SET_AND_INIT(unsigned)
    GET_SET_AND_INIT(long)
    GET_SET_AND_INIT(float)
    GET_SET_AND_INIT(double)
    GET_SET_AND_INIT(bool)

    std::map<string, string> getAllData(){
        std::map<string, string> map_string;

        ADD_TO_MAP(map_string, string)
        ADD_TO_MAP_AS_STRING(map_string, String)
        ADD_TO_MAP_AS_STRING(map_string, int)
        ADD_TO_MAP_AS_STRING(map_string, unsigned)
        ADD_TO_MAP_AS_STRING(map_string, long)
        ADD_TO_MAP_AS_STRING(map_string, float)
        ADD_TO_MAP_AS_STRING(map_string, double)
        ADD_TO_MAP_AS_STRING(map_string, bool)

        return map_string;
    }

  private:

    MAP(String)
    MAP(string)
    MAP(int)
    MAP(unsigned)
    MAP(long)
    MAP(float)
    MAP(double)
    MAP(bool)
};
