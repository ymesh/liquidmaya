C++			= CC
NO_TRANS_LINK = -no_transitive_link
CFLAGS		= -g -mips3 -n32 
C++FLAGS	= $(CFLAGS) $(WARNFLAGS) -LANG:std
INCLUDES	= -I. -I.. -I$(MAYA_LOCATION)/include -I$(RMANTREE)/include -I$(LIQUIDTREE)/include -I/usr/freeware/include
LD			= $(C++) -shared $(NO_TRANS_LINK) $(C++FLAGS)
LIBS		= -L$(MAYA_LOCATION)/lib -L$(RMANTREE)/lib -lOpenMaya -lrib -lz -lm  

.SUFFIXES: .cc .o .so .c

default: testRIBStatus

liquidFur : liquidFur.o
	$(C++) -o liquidFur.so -shared liquidFur.o $(LIBS) $(NO_TRANS_LINK) $(C++FLAGS)
	
testRIBStatus : testRIBStatus.o
	$(C++) -o testRIBStatus.so -shared testRIBStatus.o $(LIBS) $(NO_TRANS_LINK) $(C++FLAGS)

.cc.o:
	$(C++) -c $(INCLUDES) $(C++FLAGS) $<

.cc.i:
	$(C++) -E $(INCLUDES) $(C++FLAGS) $*.cc > $*.i

depend:
	makedepend $(INCLUDES) -I/usr/include/CC liquid.cc

clean:
	rm -rf *.so *.o ii_files so_locations

