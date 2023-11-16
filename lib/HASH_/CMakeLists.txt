cmake_minimum_required(VERSION 3.5)

project(HASH_ LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)

#包含的源文件和头文件
set(SOURCES
    include/HASH_.h
    src/HASH_.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})


