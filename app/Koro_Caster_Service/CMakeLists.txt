cmake_minimum_required(VERSION 3.5)

project(Koro_Caster_Service LANGUAGES CXX)

# 默认版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_EXTRA 0)

find_package(Git)

if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE PROJECT_GIT_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(PROJECT_GIT_VERSION "unknown")
endif()

# 使用git describe命令获取标签版本号
execute_process(
    COMMAND git describe --tags --dirty=-dev
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_TAG_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 检查是否成功获取了标签版本
if(NOT PROJECT_TAG_VERSION)
    message(WARNING "Failed to get Git tag.")
else()
    # 使用正则表达式提取标签中的版本号和更新次数
    string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION_REGEX "${PROJECT_TAG_VERSION}")
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
endif()

# 使用git describe命令获取更新次数
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE LATEST_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT LATEST_TAG)
    message(WARNING "Failed to get latest tag.")
else()
    # 使用git describe命令获取距离最近标签的更新次数
    execute_process(
        COMMAND git rev-list ${LATEST_TAG}..HEAD --count
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE UPDATE_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(VERSION_EXTRA ${UPDATE_COUNT})
endif()


# 设置项目的版本号
set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_EXTRA}")

set(EXE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

message(STATUS "")
message(STATUS "------- ${PROJECT_NAME} -------")

message(STATUS "DEV VERSION: ${PROJECT_VERSION}")
message(STATUS "GIT VERSION: ${PROJECT_GIT_VERSION}")
message(STATUS "TAG VERSION: ${PROJECT_TAG_VERSION}")
message(STATUS "BINARY_NAME: ${EXE_NAME}")

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
                #${EXTRA_SOURCE}
                ${VERSION_SOURCES})

#生成的可执行文件名称和包含的文件

#基于version.h.in 生成 version.h
configure_file(${CMAKE_SOURCE_DIR}/version.h.in version.h @ONLY)
target_include_directories(${EXE_NAME}  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#基于工程目录下的配置文件 生成 执行目录下的配置文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/Auth_Verify.yml.in ${CMAKE_SOURCE_DIR}/bin/Koro_Caster_Service/conf/Auth_Verify.yml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/Caster_Core.yml.in ${CMAKE_SOURCE_DIR}/bin/Koro_Caster_Service/conf/Caster_Core.yml @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/Service_Setting.yml.in ${CMAKE_SOURCE_DIR}/bin/Koro_Caster_Service/conf/Service_Setting.yml @ONLY)

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/Koro_Caster_Service")
message(STATUS "BINARY_DIR : ${CMAKE_SOURCE_DIR}/bin/Koro_Caster_Service")
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


# 如果使用的是 GCC 编译器
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   target_link_libraries(${EXE_NAME} 
                        debug       ${CMAKE_BINARY_DIR}/rely/yaml-cpp-0.8.0/libyaml-cppd.a
                        optimized   ${CMAKE_BINARY_DIR}/rely/yaml-cpp-0.8.0/libyaml-cpp.a)
endif()
# 如果使用的是 MSVC 编译器
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries(${EXE_NAME} 
                        debug       ${CMAKE_BINARY_DIR}/rely/yaml-cpp-0.8.0/yaml-cppd.lib
                        optimized   ${CMAKE_BINARY_DIR}/rely/yaml-cpp-0.8.0/libyaml-cpp.lib)
endif()
                        