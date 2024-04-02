cmake_minimum_required(VERSION 3.14)

project(rtklib LANGUAGES C)

#版本14以下可能会导致abseil不支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_RTLDLL -DNO_STRICT -DTRACE -DENAGLO -DENAQZS -DENAGAL -DENACMP -DENAIRN -DNFREQ=3 -DNEXOBS=3 -DEXTLEX )

include_directories(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/include)

set(SOURCES
    include/rtklib_global.h
    include/rtklib.h
    src/rtkcmn.c
    src/convkml.c
    src/convrnx.c
    src/convgpx.c
    src/datum.c
    src/download.c
    src/ephemeris.c
    src/geoid.c
    src/gis.c
    src/ionex.c
    src/lambda.c
    src/options.c
    src/pntpos.c
    src/postpos.c
    src/ppp.c
    src/ppp_ar.c
    src/preceph.c
    src/rcvraw.c
    src/rinex.c
    src/rtcm.c
    src/rtcm2.c
    src/rtcm3.c
    src/rtcm3e.c
    src/rtkpos.c
    src/rtksvr.c
    src/sbas.c
    src/solution.c
    src/stream.c
    src/streamsvr.c
    src/tides.c
    src/tle.c
    src/rcv/binex.c
    src/rcv/crescent.c
    src/rcv/javad.c
    src/rcv/novatel.c
    src/rcv/nvs.c
    src/rcv/rt17.c
    src/rcv/septentrio.c
    src/rcv/skytraq.c
    src/rcv/ss2.c
    src/rcv/ublox.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
target_link_libraries(${PROJECT_NAME} winmm)
ENDIF()
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

