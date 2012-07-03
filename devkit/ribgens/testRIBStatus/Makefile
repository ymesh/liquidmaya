LIQUIDHOME ?= /liquidmaya
MAYA_VERSION ?= 2009
AW_LOCATION ?= /usr/autodesk
MAYA_LOCATION ?= $(AW_LOCATION)/maya$(MAYA_VERSION)-x64
PLUG_DIR = Maya$(MAYA_VERSION)x64
LIQPLUGINS = $(LIQUIDHOME)/lib/plugins/$(PLUG_DIR)

DEPTH = .
BIN_VERSION ?= release
VPATH ?= $(VPATHRELEASE)

AR	= ar cqs
CPP = g++

CPPFLAGS = -DGENERIC_RIBLIB -DREQUIRE_IOSTREAM -D_BOOL -DNDEBUG -DLINUX -DHAVE_ZLIB -fPIC -pthread -w
EXPORT = -EXPORT:RIBGenCreate -EXPORT:RIBGenDestroy
LDFLAGS	= $(CPPFLAGS) -L$(MAYA_LOCATION)/lib -L/usr/lib64 -L$(LIQUIDHOME)/ribLib/lib 
INCLUDES = -I$(MAYA_LOCATION)/include -I$(LIQUIDHOME)/include -I$(LIQUIDHOME)/ribLib 
MAYALIBS		= -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lOpenMayaAnim -lOpenMayaFX -lGLU
LIBS				= $(MAYALIBS) -lrib -lm

OBJPATH = $(DEPTH)/obj/$(PLUG_DIR)

	
testRIBStatus.so : $(OBJPATH)/testRIBStatus.o
	$(CPP) $(LDFLAGS) -shared -o $(OBJPATH)/testRIBStatus.so $(OBJPATH)/testRIBStatus.o $(LIBS)

#.cpp.o :
#	@echo "compiling > $@"
#	@mkdir -p $(OBJPATH)
#	$(CPP) -c $(INCLUDES) $(CPPFLAGS) -o $(OBJPATH)/$@ $<

$(OBJPATH)/%.o : %.cpp
	@echo "compiling > $@"
	@mkdir -p $(OBJPATH)
	$(CPP) $(CPPFLAGS) -c $(INCLUDES) -o $@ $<

install :
	cp $(OBJPATH)/testRIBStatus.so  $(LIQPLUGINS)
	
clean :
	rm -f $(OBJPATH)/*.o

