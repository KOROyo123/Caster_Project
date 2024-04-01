cmake_minimum_required(VERSION 3.16)

project(Caster_Project)

set(CMAKE_CXX_STANDARD 20)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
add_definitions(-DLINUX)
ENDIF()
IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
add_definitions(-DWIN32)
ENDIF()





add_subdirectory(rely/)
add_subdirectory(lib/)
add_subdirectory(app/)
add_subdirectory(dev/)










