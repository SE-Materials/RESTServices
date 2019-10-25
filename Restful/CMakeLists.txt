cmake_minimum_required(VERSION 2.8.7)
project(cppservice)

set(CMAKE_CXX_STANDARD 11)

add_executable(cppservice casablanca.cpp)

target_link_libraries(cppservice PRIVATE boost_system crypto ssl cpprest libcpp_redis.a libtacopie.a pthread)

