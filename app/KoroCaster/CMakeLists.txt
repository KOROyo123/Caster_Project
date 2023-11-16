cmake_minimum_required(VERSION 3.5)

project(KoroCaster_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#包含的源文件和头文件
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src)

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/basic)


# add_definitions(-DNTRIP_MULT_THREAD)
# add_definitions(-DENABLE_TABLE_MUTEX)

set(DB_TABLE_SOURCE
    include/DB/hash_table.h
    include/DB/tcp_connect_table.h
    src/DB/tcp_connect_table.cpp
    include/DB/ntrip_client_table.h
    src/DB/ntrip_client_table.cpp
    include/DB/ntrip_server_table.h
    src/DB/ntrip_server_table.cpp
    include/DB/ntrip_subclient_table.h
    src/DB/ntrip_subclient_table.cpp
    include/DB/relay_account_table.h
    src/DB/relay_account_table.cpp
)

set(CONNECT_TYPE_SOURCE
    include/connect_type.h
    src/Connect/que_client.cpp
    src/Connect/ntrip_client.cpp
    src/Connect/ntrip_server.cpp
    src/Connect/relay_client.cpp
    src/Connect/relay_server.cpp
    src/Connect/tcp_connect.cpp
)

set(MAIN_SOURCES
    include/ntrip_global.h
    include/ntripcenter.h
    src/main.cpp
    src/ntripcenter.cpp
)

add_executable(${PROJECT_NAME} 
        ${BASIC_SOUCE}
        ${CONNECT_TYPE_SOURCE} 
        ${DB_TABLE_SOURCE}
        ${MAIN_SOURCES}
        )

#生成的可执行文件名称和包含的文件

#Koro_NavTool
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src)

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
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/rtklib-2.4.3-b34/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/rtklib-2.4.3-b34/librtklib.a)

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME}  OpenSSL::Crypto OpenSSL::SSL)

#libevent 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/libevent-2.1.12-stable/include)
#libevent 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/build/libevent-2.1.12-stable/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/build/libevent-2.1.12-stable/lib/libevent.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/build/libevent-2.1.12-stable/lib/libevent_core.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/build/libevent-2.1.12-stable/lib/libevent_extra.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/build/libevent-2.1.12-stable/lib/libevent_pthreads.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/build/libevent-2.1.12-stable/lib/libevent_openssl.a)

#Redis 
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/redis-7.2.1/deps/hiredis)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/redis-7.2.1/build/libhiredis.a)

#spdloginclude_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/rely/spdlog-1.12.0/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/rely/spdlog-1.12.0/libspdlogd.a)
