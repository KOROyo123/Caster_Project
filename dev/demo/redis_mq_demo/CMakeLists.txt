cmake_minimum_required(VERSION 3.5)

project(redis_mq_demo_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# add_definitions(-DNTRIP_MULT_THREAD)
# add_definitions(-DENABLE_TABLE_MUTEX)
set(SOURCES
main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

#rtklib
target_link_libraries(${PROJECT_NAME} rtklib)


#redismq
target_link_libraries(${PROJECT_NAME} redis_mq)

#spdlog
target_link_libraries(${PROJECT_NAME} spdlog)

#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)
target_link_libraries(${PROJECT_NAME} event_pthreads)

#Redis 
target_link_libraries(${PROJECT_NAME} hiredis)
