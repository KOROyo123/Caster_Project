cmake_minimum_required(VERSION 3.16)

project(Koro_Caster)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
add_definitions(-DLINUX)
ENDIF()
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
add_definitions(-DWIN32)
ENDIF()


add_subdirectory(lib/)
add_subdirectory(app/)







