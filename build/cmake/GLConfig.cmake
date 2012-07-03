IF( GL_INCLUDE_DIR AND GL_LIBRARIES )
  SET( GL_FIND_QUIETLY TRUE )
ENDIF( GL_INCLUDE_DIR AND GL_LIBRARIES )

#-lGLU -lGL

if(WIN32)
  set( GL_LIBRARIES glu32.lib opengl32.lib)
  set( GL_INCLUDE_DIR $ENV{INCLUDE})
endif(WIN32)

if(UNIX)
	if(APPLE)
		#set(GL_INCLUDE "/System/Library/Frameworks/AGL.framework/Headers")
		set(GL_INCLUDE "/System/Library/Frameworks/OpenGL.framework/Headers")
		#set(GL_INCLUDE "/usr/include/GL")
		FIND_PATH( GL_INCLUDE_DIR gl.h ${GL_INCLUDE} )
		# set(GL_LIB "/usr/X11/lib")
		
		set( GL_LIB "/System/Library/Frameworks/OpenGL.framework/Libraries" )
		
		message( STATUS "GL_INCLUDE = ${GL_INCLUDE}")
		message( STATUS "GL_LIB = ${GL_LIB}")
		
		#FIND_LIBRARY( GL_LIB_GL NAMES GL PATHS ${GL_LIB} )
		#FIND_LIBRARY( GL_LIB_GLU NAMES GLU PATHS ${GL_LIB} )
		#set( GL_LIBRARIES ${GL_LIB_GL} ${GL_LIB_GLU} )
		set( GL_LIBRARIES ${GL_LIB}/libGL.dylib;${GL_LIB}/libGLU.dylib )
		message( STATUS "> GL_INCLUDE_DIR = ${GL_INCLUDE_DIR}")
    message( STATUS "> GL_LIBRARIES = ${GL_LIBRARIES}")
	else(APPLE)
		set(GL_INCLUDE "/usr/include/GL")
		set(GL_LIB "/usr/X11/lib")
		FIND_PATH( GL_INCLUDE_DIR gl.h ${GL_INCLUDE} )
		FIND_LIBRARY( GL_LIB_GL NAMES GL PATHS ${GL_LIB} )
		FIND_LIBRARY( GL_LIB_GLU NAMES GLU PATHS ${GL_LIB} )
		set( GL_LIBRARIES ${GL_LIB_GL} ${GL_LIB_GLU})
	endif(APPLE)
endif(UNIX)

INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( GL DEFAULT_MSG GL_LIBRARIES GL_INCLUDE_DIR )


MARK_AS_ADVANCED( GL_INCLUDE_DIR GL_LIBRARIES )

