cmake_minimum_required(VERSION 3.5)

project(castercore LANGUAGES CXX)
# project(qpidclient LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions("-g")

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)

set(SOURCES
    include/Caster_Core.h
    src/Caster_Core.cpp
    src/caster_core_internal.h
    src/caster_core_internal.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
#libevent 
target_link_libraries(${PROJECT_NAME} event)
target_link_libraries(${PROJECT_NAME} event_core)
target_link_libraries(${PROJECT_NAME} event_extra)


#hiredis 
target_link_libraries(${PROJECT_NAME} hiredis)

#json
include_directories(${EXE_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)

#spdlog
add_definitions(-DUSE_SPD_LOG)
target_link_libraries(${PROJECT_NAME} spdlog)




