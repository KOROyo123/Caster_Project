cmake_minimum_required(VERSION 3.5)

project(redis_mq LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)


set(SOURCES
    include/MsgSwitch.h
    src/MsgSwitch.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

#spdlog
target_link_libraries(${PROJECT_NAME} spdlog)

#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)
target_link_libraries(${PROJECT_NAME} event_pthreads)
#Redis 
target_link_libraries(${PROJECT_NAME} hiredis)

