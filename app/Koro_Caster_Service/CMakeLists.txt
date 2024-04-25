cmake_minimum_required(VERSION 3.5)

project(Koro_Caster_Service VERSION 1.0 LANGUAGES CXX)

find_package(Git)

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty=-dev
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECT_TAG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE PROJECT_GIT_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )

else()
    set(PROJECT_GIT_VERSION "unknown")
    set(PROJECT_TAG_VERSION "unknown")
endif()


set(EXE_NAME "${PROJECT_NAME}-${PROJECT_TAG_VERSION}")

message(STATUS "")
message(STATUS "------- ${PROJECT_NAME} -------")

message(STATUS "Dev version: ${PROJECT_VERSION}")
message(STATUS "Git version: ${PROJECT_GIT_VERSION}")
message(STATUS "TAG version: ${PROJECT_TAG_VERSION}")
message(STATUS "EXE  name  : ${EXE_NAME}")

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#包含的源文件和头文件
include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Carrier)
include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Compontent)
include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Connector)
include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/DB)
include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Extra)

include_directories(${EXE_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)

 set(DB_TABLE_SOURCE
    include/DB/relay_account_tb.h
    src/DB/relay_account_tb.cpp
)
set(COMPONTENT_TYPE_SOURCE
    include/Compontent/process_queue.h
    src/Compontent/process_queue.cpp
)

set(CARRIER_TYPE_SOURCE
    include/Carrier/client_ntrip.h
    include/Carrier/server_ntrip.h
    include/Carrier/server_relay.h
    src/Carrier/client_ntrip.cpp
    src/Carrier/server_ntrip.cpp
    src/Carrier/server_relay.cpp
    include/Carrier/source_ntrip.h
    src/Carrier/source_ntrip.cpp
)

set(CONNECTOR_TYPE_SOURCE
    include/Connector/ntrip_compat_listener.h
    include/Connector/ntrip_relay_connector.h
    src/Connector/ntrip_compat_listener.cpp
    src/Connector/ntrip_relay_connector.cpp
)

set(EXTRA_SOURCE
    include/Extra/heart_beat.h
    src/Extra/heart_beat.cpp
)

set(MAIN_SOURCES
    include/ntrip_global.h
    include/ntrip_caster.h
    src/main.cpp
    src/ntrip_caster.cpp
)

add_executable(${EXE_NAME} 
                ${MAIN_SOURCES} 
                ${CARRIER_TYPE_SOURCE}  
                ${CONNECTOR_TYPE_SOURCE}
                ${COMPONTENT_TYPE_SOURCE}    
                ${DB_TABLE_SOURCE} 
                ${EXTRA_SOURCE}
                ${VERSION_SOURCES})

#生成的可执行文件名称和包含的文件

#基于version.h.in 生成 version.h
configure_file(${CMAKE_SOURCE_DIR}/version.h.in version.h @ONLY)
target_include_directories(${EXE_NAME}  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#基于工程目录下的配置文件 生成 执行目录下的配置文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/Auth_Verify.yml ${CMAKE_CURRENT_BINARY_DIR}/conf/Auth_Verify.yml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/Caster_Core.yml ${CMAKE_CURRENT_BINARY_DIR}/conf/Caster_Core.yml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/Service_Setting.yml ${CMAKE_CURRENT_BINARY_DIR}/conf/Service_Setting.yml @ONLY)



#Koro_NavTool
target_link_libraries(${EXE_NAME} knt)
#caster_core
target_link_libraries(${EXE_NAME} castercore)
#auth_verify
target_link_libraries(${EXE_NAME} authverify)

#libevent 
target_link_libraries(${EXE_NAME} event)
target_link_libraries(${EXE_NAME} event_core)
target_link_libraries(${EXE_NAME} event_extra)
#nlohmann/json
include_directories(${EXE_NAME} ${JSON_INCLUDE_PATH})
#spdlog
target_link_libraries(${EXE_NAME} spdlog)
#yaml

include_directories(${EXE_NAME} ${CMAKE_SOURCE_DIR}/rely/yaml-cpp-0.8.0/include)
target_link_libraries(${EXE_NAME} 
                        debug       ${CMAKE_BINARY_DIR}/rely/yaml-cpp-0.8.0/libyaml-cppd.a
                        optimized   ${CMAKE_BINARY_DIR}/rely/yaml-cpp-0.8.0/libyaml-cpp.a)



