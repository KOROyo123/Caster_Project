cmake_minimum_required(VERSION 3.5)

project(Koro_Project_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/core)


set(CORE_SOURCES
    core/cs_core.h
    core/cs_core.cpp
)

set(SOURCES
    include/cs_cui.h
    src/cs_cui.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${CORE_SOURCES})

#Json-develop
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)
