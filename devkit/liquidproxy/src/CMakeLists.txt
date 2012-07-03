cmake_minimum_required(VERSION 2.6.4)

set(TARGET_NAME "liquidproxy")

if("$ENV{ARCH}" STREQUAL "-x64")
  set(TARGET_NAME "${TARGET_NAME}$ENV{ARCH}")  
endif("$ENV{ARCH}" STREQUAL "-x64")

project(${TARGET_NAME})

set(CMAKE_BUILD_TYPE "Release")

if(WIN32)
	add_definitions(-DWINNT)
	add_definitions(-D_WIN32)
endif(WIN32)

file(GLOB src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

include_directories( "$ENV{BOOST_ROOT}" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" ) 

message( STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message( STATUS "TARGET_NAME = ${TARGET_NAME}")

message( STATUS "SOURCE_DIR = ${${TARGET_NAME}_SOURCE_DIR}")
message( STATUS "BINARY_DIR = ${${TARGET_NAME}_BINARY_DIR}")

add_definitions( -DNDEBUG )
add_definitions( -D_BOOL )
add_definitions( -DREQUIRE_IOSTREAM ) 
add_definitions( -D_CRT_SECURE_NO_DEPRECATE )

add_executable(${TARGET_NAME} liquidproxy.cpp )
set_target_properties( ${TARGET_NAME} PROPERTIES OUTPUT_NAME "${TARGET_NAME}" )

#if(UNIX AND NOT APPLE)
#if(UNIX)
   set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-w -fPIC -pthread")
#else(UNIX)
  set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-w")
#endif(UNIX)
#endif(UNIX AND NOT APPLE)


target_link_libraries( ${TARGET_NAME} )
install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin )
