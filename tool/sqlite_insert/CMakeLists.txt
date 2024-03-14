cmake_minimum_required(VERSION 3.5)

project(sqlite_insert_v1.0 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)


set(SOURCES
include/sqlite_insert.h
src/sqlite_insert.cpp
src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

#spdlog
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/build/libspdlog.a)

#Json-develop
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)

#sqlite
target_link_libraries(${PROJECT_NAME} sqlite3) 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/sqlite-amalgamation-3440000)
