
project(assumed_fill_test CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DJSON_NOEXCEPTION=1 -DJSON_USE_IMPLICIT_CONVERSIONS=0")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DJSON_USE_IMPLICIT_CONVERSIONS=0")
add_executable(assumed_fill_test main.cpp GameItem.cpp Location.cpp LocationManager.cpp Setting.cpp)