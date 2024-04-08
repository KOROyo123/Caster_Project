cmake_minimum_required(VERSION 3.5)

project(Koro_Caster_Service VERSION 0.1.0 LANGUAGES CXX)

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
    include/Compontent/auth_verifier.h
    src/Compontent/auth_verifier.cpp
    include/Compontent/data_transfer.h
    src/Compontent/data_transfer.cpp
    include/Compontent/source_transfer.h
    src/Compontent/source_transfer.cpp
    include/Compontent/file_loger.h
    src/Compontent/file_loger.cpp
    include/Compontent/caster_core.h
    src/Compontent/caster_core.cpp
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
    include/Extra/smtp_ssl.h
    src/Extra/smtp_ssl.cpp
)

set(MAIN_SOURCES
    include/ntrip_global.h
    include/ntrip_caster.h
    include/process_queue.h
    src/process_queue.cpp
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

#基于 version.h.in 生成 version.h
configure_file(${CMAKE_SOURCE_DIR}/version.h.in version.h @ONLY)
target_include_directories(${EXE_NAME}  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#基于 工程目录下的破i之文件 生成 执行目录下的配置文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Koro_Caster_Service_Conf.json ${CMAKE_CURRENT_BINARY_DIR}/Koro_Caster_Service_Conf.json @ONLY)

#Koro_NavTool
target_link_libraries(${EXE_NAME} knt)

#Json-develop
include_directories(${EXE_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)

#如果是windows环境，则rtklib还需要加两个库
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(${EXE_NAME} Ws2_32 winmm)
ENDIF()
#如果是linux环境，需要手动链接math.h pthread库
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${EXE_NAME} m pthread)
ENDIF()

#rtklib
target_link_libraries(${EXE_NAME} rtklib)

#openssl (找不到就安装一下 sudo apt install libssl-dev)
# IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
# #windows版本(是否有效还需要验证）
#     set(OPENSSL_ROOT_DIR "C:/ProgramFiles/OpenSSL-Win64")
#     set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
#     set(OPENSSL_LIBRARIES "${OPENSSL_ROOT_DIR}/lib/libssl_static.lib"
#                           "${OPENSSL_ROOT_DIR}/lib/libcrypto_static.lib")
#     include_directories(${OPENSSL_INCLUDE_DIR})
#     target_link_libraries(${EXE_NAME} ${OPENSSL_LIBRARIES})
# ENDIF()
# IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
#     find_package(OpenSSL REQUIRED)
#     target_link_libraries(${EXE_NAME}  OpenSSL::Crypto OpenSSL::SSL)
#     # 采用自编译的方式解决openeular版本找不到ssl库的问题
#     # include_directories(${EXE_NAME} ${CMAKE_SOURCE_DIR}/rely/openssl-3.0.12/include)
#     # target_link_libraries(${EXE_NAME} ${CMAKE_SOURCE_DIR}/rely/openssl-3.0.12/build/libcrypto.a)
#     # target_link_libraries(${EXE_NAME} ${CMAKE_SOURCE_DIR}/rely/openssl-3.0.12/build/libssl.a)
# ENDIF()

#libevent 
target_link_libraries(${EXE_NAME} event)
target_link_libraries(${EXE_NAME} event_core)
target_link_libraries(${EXE_NAME} event_extra)

#hiredis 
target_link_libraries(${EXE_NAME} hiredis)

#spdlog
target_link_libraries(${EXE_NAME} spdlog)




