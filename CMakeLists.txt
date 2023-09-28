cmake_minimum_required(VERSION 3.2)
project(Demo)
SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

SET(SRC_FILES main.cpp 
    threadnotify.cpp
    toolFun.cpp)
#target_include_directories(HttpServer PRIVATE ${CMAKE_SOURCE_DIR})

add_executable(HttpServer ${SRC_FILES})
target_link_libraries(HttpServer PRIVATE Threads::Threads)