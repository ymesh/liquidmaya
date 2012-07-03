cmake_minimum_required(VERSION 2.6.4)

set(TARGET_NAME $ENV{LIQDPYNAME})

set(TARGET_DIR $ENV{LIQRMAN})
#if("$ENV{ARCH}" STREQUAL "-x64")
#  set(TARGET_DIR "${TARGET_DIR}$ENV{ARCH}")  
#endif("$ENV{ARCH}" STREQUAL "-x64")

link_directories( "$ENV{LIQRMANLIB}" )

project(${TARGET_NAME})
set(CMAKE_BUILD_TYPE "Release")
#set(BUILD_SHARED_LIBS true) 
#set(CMAKE_SHARED_LIBRARY_SUFFIX $ENV{LIQDPYSUFFIX} )

#find_package(zlib REQUIRED)

if(UNIX)
	if(APPLE)
		if("$ENV{ARCH}" STREQUAL "-x64")
		  set(CMAKE_OSX_ARCHITECTURES "x86_64")
		else("$ENV{ARCH}" STREQUAL "-x64")
		  set(CMAKE_OSX_ARCHITECTURES "i386")
		endif("$ENV{ARCH}" STREQUAL "-x64")
		#Changing the order of configurations to be sure release is the first one
		set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")
		add_definitions(-DMACOSX)
	else(APPLE)
		add_definitions(-DLINUX)
	endif(APPLE)
endif(UNIX)
if(WIN32)
	add_definitions(-DWINNT)
endif(WIN32)

set(src $ENV{LIQDPYSRC})

#file(GLOB src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
#file(GLOB inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" ) 
include_directories( "$ENV{LIQUIDHOME}/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )
include_directories( "$ENV{LIQRMANINC}" )


add_definitions( -DNDEBUG )
add_definitions( -D_BOOL )
add_definitions( -D_USRDLL )
add_definitions( -DREQUIRE_IOSTREAM )
#add_definitions( -DDISPLAYDRIVERPRMAN_EXPORTS )
add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
add_definitions( $ENV{LIQRMANFLAGS}  ) 
#add_definitions( -D_LIB )
#add_definitions( -DHAVE_ZLIB )
#add_definitions( -DYY_NO_UNISTD_H )

add_library( ${TARGET_NAME} SHARED ${src} )
set_target_properties( ${TARGET_NAME} PROPERTIES PREFIX "" OUTPUT_NAME "${TARGET_NAME}" SUFFIX $ENV{LIQDPYSUFFIX} )
target_link_libraries( ${TARGET_NAME} $ENV{LIQRMANLIBS} $ENV{LIBS} )


message( STATUS "TARGET_NAME = ${TARGET_NAME}<")
message( STATUS "TARGET_DIR = ${TARGET_DIR}<")
message( STATUS "SOURCE_DIR = ${${TARGET_NAME}_SOURCE_DIR}")
message( STATUS "BINARY_DIR = ${${TARGET_NAME}_BINARY_DIR}")
message( STATUS "LIQRMANLIB = $ENV{LIQRMANLIB}")
message( STATUS "src = ${src}")

if(UNIX AND NOT APPLE)
   set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-fPIC -pthread")
endif(UNIX AND NOT APPLE)

if(UNIX)
	install( TARGETS ${TARGET_NAME} LIBRARY DESTINATION ${TARGET_DIR} )
else(UNIX)
	install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${TARGET_DIR} )
endif(UNIX)
