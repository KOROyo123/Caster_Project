cmake_minimum_required(VERSION 3.5)

project(NtripCenter_0.0.1 LANGUAGES CXX)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#包含的源文件和头文件
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src)
set(MAIN_SOURCES
    src/con_table.h
    src/con_table.cpp
    src/main.cpp
    src/ntrip_global.h
    src/ntripcenter.cpp
    src/ntripcenter.h
    src/verify_info.cpp
    src/verify_info.h
    src/ht-internal.h
)

set(CONNECT_TYPE_SOURCE
    src/connect_type.h
    src/que_client.cpp
    src/ntrip_client.cpp
    src/ntrip_server.cpp
    src/relay_client.cpp
    src/relay_server.cpp
    src/tcp_connect.cpp
)

add_executable(${PROJECT_NAME}  ${SOURCE})


#生成的可执行文件名称和包含的文件

#rtklib
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/rtklib-2.4.3-b34/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/rtklib-2.4.3-b34/librtklib.a)
#如果是windows环境，则rtklib还需要加两个库
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(${PROJECT_NAME} Ws2_32 winmm)
ENDIF()
#如果是linux环境，需要手动链接math.h pthread库
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME} m pthread)
ENDIF()


#libevent 源文件
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/libevent-2.1.12-stable/)
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/libevent-2.1.12-stable/include)
#libevent 静态库
include_directories(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/libevent-2.1.12-stable/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/libevent-2.1.12-stable/lib/libevent.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/libevent-2.1.12-stable/lib/libevent_core.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/libevent-2.1.12-stable/lib/libevent_extra.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/third/libevent-2.1.12-stable/lib/libevent_pthreads.a)



# #abseil 
# # Process Abseil's CMake build system
# #add_subdirectory(abseil-cpp)
# include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third/abseil-cpp-20230802.1/)
# # Declare dependency on the absl::strings library
# target_link_libraries(${PROJECT_NAME}  absl::strings)

#Public
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/Public/include)

#CCDKGProtoDecoding
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/CDKGProtoDecoding/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/CDKGProtoDecoding/libCDKGProtoDecoding.a)

#CommonTool
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/CommonTool/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/CommonTool/libCommonTool.a)

#Concurrent
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/Concurrent/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/Concurrent/libConcurrent.a)

#CtrlCenter
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/CtrlCenter/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/CtrlCenter/libCtrlCenter.a)

#NetHandle
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/NetHandle/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/NetHandle/libNetHandle.a)

#TinyLog
include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/TinyLog/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/lib/TinyLog/libTinyLog.a)



#ifdef WIN32

#else

#endif
