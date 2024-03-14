cmake_minimum_required(VERSION 3.5)

project(Koro_Caster_Service_0.0.5 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#包含的源文件和头文件
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Carrier)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Compontent)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Connector)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/DB)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include/Extra)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)


# add_definitions(-DNTRIP_MULT_THREAD)
# add_definitions(-DENABLE_TABLE_MUTEX)

 set(DB_TABLE_SOURCE
    include/DB/relay_account_tb.h
    src/DB/relay_account_tb.cpp
)
set(COMPONTENT_TYPE_SOURCE
    include/Compontent/auth_verifier.h
    src/Compontent/auth_verifier.cpp
    include/Compontent/data_transfer.h
    src/Compontent/data_transfer.cpp
    include/Compontent/client_source.h
    src/Compontent/client_source.cpp
    include/Compontent/file_loger.h
    src/Compontent/file_loger.cpp
)


set(CARRIER_TYPE_SOURCE
    include/Carrier/client_ntrip.h
    include/Carrier/server_ntrip.h
    include/Carrier/server_relay.h
    src/Carrier/client_ntrip.cpp
    src/Carrier/server_ntrip.cpp
    src/Carrier/server_relay.cpp
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

add_executable(${PROJECT_NAME} 
                ${MAIN_SOURCES} 
                ${CARRIER_TYPE_SOURCE}  
                ${CONNECTOR_TYPE_SOURCE}
                ${COMPONTENT_TYPE_SOURCE}    
                ${DB_TABLE_SOURCE} 
                ${EXTRA_SOURCE})

#生成的可执行文件名称和包含的文件

#Koro_NavTool
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/KNT/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/KNT/libknt.a)

#Json-develop
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/json-3.11.2/include)

#如果是windows环境，则rtklib还需要加两个库
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(${PROJECT_NAME} Ws2_32 winmm)
ENDIF()
#如果是linux环境，需要手动链接math.h pthread库
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME} m pthread)
ENDIF()

#rtklib
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/rtklib-2.4.3-b34/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/rtklib-2.4.3-b34/build/librtklib.a)


#openssl (找不到就安装一下 sudo apt install libssl-dev)
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME}  OpenSSL::Crypto OpenSSL::SSL)
# 采用自编译的方式解决openeular版本找不到ssl库的问题
# include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/openssl-3.0.12/include)
# target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/openssl-3.0.12/build/libcrypto.a)
# target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/openssl-3.0.12/build/libssl.a)

#libevent 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/include)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_core.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_extra.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_pthreads.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/build/lib/libevent_openssl.a)

#Redis 
# include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/redis-7.2.1/deps/hiredis)
#target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/redis-7.2.1/build/libhiredisd.a)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/hiredis-1.2.0)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/rely/hiredis-1.2.0/libhiredis.a)

#spdlog
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/build/libspdlog.a)

#curl
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/curl-curl-8_4_0/include)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/curl-curl-8_4_0/build/lib)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/curl-curl-8_4_0/build/lib/libcurl.a)
