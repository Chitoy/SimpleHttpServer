cmake_minimum_required(VERSION 3.2)
project(Demo)
SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
# 手动指定Curl库路径和头文件路径
find_path(CURL_INCLUDE_DIRS NAMES curl/curl.h)
find_library(CURL_LIBRARIES NAMES curl)

SET(SRC_FILES main.cpp 
    threadnotify.h
    threadnotify.cpp
    toolFun.h
    toolFun.cpp
    Chatgpt.h
    Chatgpt.cpp
    EpollServer.h
    EpollServer.cpp
    HttpServer.h
    HttpServer.cpp
    )

#target_include_directories(HttpServer PRIVATE ${CMAKE_SOURCE_DIR})
add_subdirectory(json)

add_executable(HttpServer ${SRC_FILES})

target_include_directories(HttpServer PRIVATE ${CURL_INCLUDE_DIRS})

target_link_libraries(HttpServer PRIVATE Threads::Threads ${CURL_LIBRARIES})
target_link_libraries(HttpServer PRIVATE nlohmann_json::nlohmann_json)
