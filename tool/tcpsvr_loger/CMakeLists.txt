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
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/build/libspdlog.a)

#libevent 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/include)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_core.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_extra.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_pthreads.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_openssl.a)