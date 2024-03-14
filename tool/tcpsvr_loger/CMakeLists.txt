cmake_minimum_required(VERSION 3.5)

project(tcpsvr_loger_v1.0 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)


set(SOURCES
include/tcpsvr_loger.h
src/tcpsvr_loger.cpp
src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

#spdlog
target_link_libraries(${PROJECT_NAME} spdlog)

#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)