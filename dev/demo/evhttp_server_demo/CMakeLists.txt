cmake_minimum_required(VERSION 3.5)

project(evhttp_server_demo_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#包含的源文件和头文件
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)

# add_definitions(-DNTRIP_MULT_THREAD)
# add_definitions(-DENABLE_TABLE_MUTEX)
set(SOURCES
    include/http_server.h
    src/main.cpp
    src/http_server.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})


#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)