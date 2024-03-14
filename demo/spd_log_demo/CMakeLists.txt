cmake_minimum_required(VERSION 3.5)

project(spd_log_demo_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# add_definitions(-DNTRIP_MULT_THREAD)
# add_definitions(-DENABLE_TABLE_MUTEX)
set(SOURCES
main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})


#spdlog
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/build/libspdlog.a)
