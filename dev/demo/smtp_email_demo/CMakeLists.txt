cmake_minimum_required(VERSION 3.5)

project(smtp_email_demo_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#包含的源文件和头文件
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)



set(MAIN_SOURCES
    include/smtp_email.h
    include/smtp_ssl.h
    src/main.cpp
    src/smtp_email.cpp
    src/smtp_ssl.cpp
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})


#Koro_NavTool
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/KNT/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/KNT/libknt.a)

#Json-develop
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)


#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)



#openssl
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME}  OpenSSL::Crypto OpenSSL::SSL)  