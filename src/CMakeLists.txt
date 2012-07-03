cmake_minimum_required(VERSION 2.6.4)

set(TARGET_NAME liquid)
set(TARGET_DIR $ENV{LIQRMAN})
if(UNIX)
	if(APPLE)
		set(TARGET_DIR $ENV{LIQRMAN}/osx)
	else(APPLE)
		set(TARGET_DIR $ENV{LIQRMAN}/linux)
	endif(APPLE)
else(UNIX)
	set(TARGET_DIR $ENV{LIQRMAN}/win)		
endif(UNIX)

#if("$ENV{ARCH}" STREQUAL "-x64")
#  set(TARGET_DIR "${TARGET_DIR}$ENV{ARCH}")  
#endif("$ENV{ARCH}" STREQUAL "-x64")

project(${TARGET_NAME})
set(CMAKE_BUILD_TYPE "Release")

set(Maya_DIR $ENV{LIQUIDHOME}/build/cmake)
find_package(Maya REQUIRED)

message( STATUS "MAYA_LOCATION = $ENV{MAYA_LOCATION}")
message( STATUS "Maya_INCLUDE_DIR = ${Maya_INCLUDE_DIR}")
message( STATUS "MAYA_LIB = ${MAYA_LIB}")
message( STATUS "Maya_LIBRARIES = ${Maya_LIBRARIES}")

if(UNIX)
set(ZLIB_INCLUDE_DIR "/usr/include")
set(ZLIB_INCLUDE_DIRS "/usr/include")
set(ZLIB_LIBRARIES "/usr/lib64")

	if("$ENV{MAYA_VERSION}" STREQUAL "2012" )
		set(ZLIB_LIBRARIES ${MAYA_LIB} )
	endif("$ENV{MAYA_VERSION}" STREQUAL "2012")

endif(UNIX)

set(zlib_DIR $ENV{LIQUIDHOME}/build/cmake)
find_package( zlib REQUIRED )

message( STATUS "ZLIB_INCLUDE_DIR = ${ZLIB_INCLUDE_DIR}")
message( STATUS "ZLIB_LIBRARIES = ${ZLIB_LIBRARIES}")
#message( STATUS "ZLIB_LIBRARIES = $ENV{ZLIB_LIBRARIES}")

set(GL_DIR $ENV{LIQUIDHOME}/build/cmake)
find_package(GL REQUIRED)
message( STATUS "GL_INCLUDE_DIR = ${GL_INCLUDE_DIR}")
message( STATUS "GL_LIBRARIES = ${GL_LIBRARIES}")

if(UNIX)
	if(APPLE)
		#set(CMAKE_OSX_ARCHITECTURES "i386;x86_64;")
		if("$ENV{ARCH}" STREQUAL "-x64")
		  set(CMAKE_OSX_ARCHITECTURES "x86_64")
		else("$ENV{ARCH}" STREQUAL "-x64")
		  set(CMAKE_OSX_ARCHITECTURES "i386")
		endif("$ENV{ARCH}" STREQUAL "-x64")
		
		#Changing the order of configurations to be sure release is the first one
		set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")
		add_definitions(-D__APPLE__)
		add_definitions(-DMACOSX)
		add_definitions(-DOSX)
		add_definitions(-DOSMac_)
		include_directories(${GL_INCLUDE_DIR})
		include_directories("/usr/X11/include")
		include_directories("/usr/include/malloc")
		# set(EXTRA_LIBS "/usr/X11/lib")
		# set(EXTRA_LIBS "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries")
		set(EXTRA_LIBS "")
	else(APPLE)
		add_definitions(-DLINUX)
		set(EXTRA_LIBS "/usr/lib64:/lib64")
		#set(LIQUIDBINLIBS "-lawxml2 -lsvml -liff -lpthread")
		# fedora 13 needs -ldl !!!
		set(LIQUIDBINLIBS "-lawxml2 -lsvml -liff -lpthread -ldl")

		if("$ENV{MAYA_VERSION}" STREQUAL "2012" )
			set( Maya_LIBRARIES "${Maya_LIBRARIES};${MAYA_LOCATION}/lib/libadp_core-2_5.so" )
		endif("$ENV{MAYA_VERSION}" STREQUAL "2012")

    #-lguide
    set(LIQUIDBINLIBS "${LIQUIDBINLIBS} -lOpenMayalib -lAnimSlice -lDeformSlice -lDynSlice -lKinSlice -lModelSlice -lNurbsSlice -lPolySlice -lProjectSlice -lImage -lImageUI -lShared -lTranslators") 
    set(LIQUIDBINLIBS "${LIQUIDBINLIBS} -lDataModel -lNurbsEngine -lRenderModel -lDependEngine -lCommandEngine -lAG -lAnimEngine -lAppVersion -lGeometryEngine -lDependCommand -lExtensionLayer -lPolyEngine -lSubdivEngine") 
    set(LIQUIDBINLIBS "${LIQUIDBINLIBS} -lSubdivGeom -l3dGraphics -lNurbs -lPoly -lSubdiv -lHWFoundation -lHWGL -lHWRender -lBase -lSharedUI -lHWRenderMaya -lDevices -lManips -lAnimUISlice -lDynUISlice -lRenderSlice -lModelUISlice -lUrchinSlice")
	endif(APPLE)
endif(UNIX)
if(WIN32)
	add_definitions(-DWINNT)
	add_definitions(-D_WIN32)
	add_definitions(-DZLIB_WINAPI)
	add_definitions(-DZLIB_DLL )
	#add_definitions(-Dulong="unsigned long")
	#add_definitions(-Duint="unsigned int")
endif(WIN32)

file(GLOB src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp")
file(GLOB inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "$ENV{LIQUIDHOME}/include/*.h")

#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" ) 
include_directories( "$ENV{LIQUIDHOME}/include" )
include_directories( "${Maya_INCLUDE_DIR}" )
include_directories( "$ENV{BOOST_ROOT}" )
include_directories( "$ENV{LIQRMANINC}" )
include_directories( "${ZLIB_INCLUDE_DIR}" )

message( STATUS "TARGET_NAME = ${TARGET_NAME}")
message( STATUS "TARGET_DIR = ${TARGET_DIR}")

message( STATUS "SOURCE_DIR = ${${TARGET_NAME}_SOURCE_DIR}")
message( STATUS "BINARY_DIR = ${${TARGET_NAME}_BINARY_DIR}")
message( STATUS "LIQRMANLIBS = $ENV{LIQRMANLIB} $ENV{LIQRMANLIBS}")
message( STATUS "LIBS = $ENV{LIBS}")
message( STATUS "EXTRA_LIBS = ${EXTRA_LIBS}")
message( STATUS "BOOST_ROOT = $ENV{BOOST_ROOT}")

message( STATUS "LIQUIDHOME = $ENV{LIQUIDHOME}")
message( STATUS "LIQUIDVERSION = $ENV{LIQUIDVERSION}")

add_definitions( -DNDEBUG )
add_definitions( -D_BOOL )
add_definitions( -DREQUIRE_IOSTREAM )
add_definitions( -DLIQUID_EXPORTS )
add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
add_definitions( $ENV{LIQRMANFLAGS} ) 

add_library( liquidobj STATIC ${src} )

link_directories( ${MAYA_LIB} "$ENV{LIQRMANLIB}" ${EXTRA_LIBS})
add_library( liquid MODULE liquidPlug.cpp )
set_target_properties( liquid PROPERTIES PREFIX "" OUTPUT_NAME "liquid" SUFFIX "$ENV{LIQPLUGSUFFIX}" )

link_directories( ${MAYA_LIB} "$ENV{LIQRMANLIB}" ${EXTRA_LIBS} )
add_executable( liquidbin liquidBin.cpp )
set_target_properties( liquidbin PROPERTIES OUTPUT_NAME "liquid" )

if(UNIX)
	if(APPLE)
		add_definitions( -DLIQUIDVERSION="$ENV{LIQUIDVERSION}" )
		set_target_properties(liquidobj PROPERTIES COMPILE_FLAGS "-w -pthread -fno-inline")
		set_target_properties(liquid PROPERTIES COMPILE_FLAGS "-D_USRDLL -w -fPIC -pthread -fno-inline")
		set_target_properties(liquidbin PROPERTIES COMPILE_FLAGS "-DLIQUIDBIN -w -fPIC -pthread -fno-inline" ) 
		#  LINK_FLAGS "-O" LINK_FLAGS "-L/usr/lib64"
	else(APPLE)
		add_definitions( -DLIQUIDVERSION="$ENV{LIQUIDVERSION}" )
		set_target_properties(liquidobj PROPERTIES COMPILE_FLAGS "-w -fPIC -pthread -fno-inline")
		set_target_properties(liquid PROPERTIES COMPILE_FLAGS "-D_USRDLL -w -fPIC -pthread -fno-inline")
		set_target_properties(liquidbin PROPERTIES COMPILE_FLAGS "-DLIQUIDBIN -w -fPIC -pthread -fno-inline" ) 
		#LINK_FLAGS "-O" LINK_FLAGS "-L/usr/lib64"
	endif(APPLE)
else(UNIX)
   set_target_properties(liquidobj PROPERTIES COMPILE_FLAGS "-w")
   set_target_properties(liquid PROPERTIES COMPILE_FLAGS "-D_USRDLL -w")
   set_target_properties(liquidbin PROPERTIES COMPILE_FLAGS "-DLIQUIDBIN -w") 
endif(UNIX)

# ${ZLIB_LIBRARIES}
target_link_libraries( liquid liquidobj $ENV{LIQRMANLIBS} ${Maya_LIBRARIES} $ENV{LIBS} ${GL_LIBRARIES} ${ZLIB_LIBRARIES})
target_link_libraries( liquidbin liquidobj $ENV{LIQRMANLIBS} ${Maya_LIBRARIES} $ENV{LIBS} ${LIQUIDBINLIBS} ${GL_LIBRARIES} ${ZLIB_LIBRARIES})
install( TARGETS liquid liquidbin 
         LIBRARY DESTINATION ${TARGET_DIR} 
         RUNTIME DESTINATION ${TARGET_DIR} )


