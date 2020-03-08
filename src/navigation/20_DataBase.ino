#include <map>
#include <string>
using namespace std;

#define VAR_NAME(type) DB##type

#define MAP_TYPE(type) std::map<std::string, type>

#define GET_AND_SET(type)                                             \
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

    GET_AND_SET(String)
    GET_AND_SET(string)
    GET_AND_SET(int)
    GET_AND_SET(unsigned)
    GET_AND_SET(long)
    GET_AND_SET(float)
    GET_AND_SET(double)
    GET_AND_SET(bool)

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

