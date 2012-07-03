
DEPTH = .
BIN_VERSION ?= release
VPATH ?= $(VPATHRELEASE)

AR	= ar cqs
CPP = g++

CPPFLAGS = -DREQUIRE_IOSTREAM -D_BOOL -DNDEBUG -DLINUX -DHAVE_ZLIB -fPIC -pthread -w
INCLUDES = 
OBJPATH = $(DEPTH)/obj

RIBLIB := $(DEPTH)/lib/librib.a
RIBLIBOBJS = ri.o\
	riInterface.o\
	ribOut.o\
	variable.o\
	error.o

RIBLIBMAINOBJS := $(patsubst %,$(OBJPATH)/%,$(RIBLIBOBJS))
	
	
all : $(RIBLIB)

$(RIBLIB) : $(RIBLIBMAINOBJS)
	@mkdir -p $(DEPTH)/lib/
	$(AR) $(RIBLIB) $(RIBLIBMAINOBJS)

.cpp.o :
	@mkdir -p $(OBJPATH)
	$(CPP) -c $(INCLUDES) $(CPPFLAGS) -o $(OBJPATH)/$@ $<

$(OBJPATH)/%.o : %.cpp
	@mkdir -p $(OBJPATH)
	$(CPP) -c $(INCLUDES) $(CPPFLAGS) -o $@ $<
	
clean :
	rm -f $(OBJPATH)/*.o
	rm -f $(RIBLIB)
