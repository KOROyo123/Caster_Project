cmake_minimum_required(VERSION 3.5)

project(redis_push_info LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# add_definitions(-DNTRIP_MULT_THREAD)
# add_definitions(-DENABLE_TABLE_MUTEX)
set(SOURCES
redis_info_push.h
redis_info_push.cpp
main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# #redismq
# target_link_libraries(${PROJECT_NAME} redis_mq)

#spdlog
target_link_libraries(${PROJECT_NAME} spdlog)

#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)
target_link_libraries(${PROJECT_NAME} event_pthreads)

#Redis 
target_link_libraries(${PROJECT_NAME} hiredis)

#Json-develop
include_directories(${EXE_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)