cmake_minimum_required(VERSION 3.5)

project(psevent LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)

#包含的源文件和头文件
set(SOURCES
    include/ps_event.h
    src/ps_event.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})


#libevent 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/include)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_core.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_extra.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_pthreads.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_openssl.a)