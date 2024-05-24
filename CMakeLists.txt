cmake_minimum_required(VERSION 3.16)

project(Caster_Project)

# 设置使用的 C++ 标准版本为 C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  #要求编译器支持所选的标准
set(CMAKE_CXX_EXTENSIONS ON)  #启用编译器的扩展特性


IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
add_definitions(-DLINUX)
ENDIF()
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
add_definitions(-DWIN32)
ENDIF()

set(JSON_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/third/json-3.11.3/include")
set(SPDLOG_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/include")

#全局设置选项
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)

add_subdirectory(rely/)
add_subdirectory(lib/)
add_subdirectory(app/)
add_subdirectory(dev/)










