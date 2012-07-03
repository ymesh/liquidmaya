# Common definition to compile shaders
DEPTH = ..
include $(DEPTH)/commondefs.mk

INCLUDES 	= -I./include -I$(LIQRMANPATH)/include

SLSHADERS	= $(wildcard src/*.sl)
SLOSHADERS	= $(SLSHADERS:.sl=.$(SLOEXT))

SRCS			= $(SLSHADERS)
VPATHDEBUG		= $(DEPTH)/bin/$(VBIN)/debug/$(LIQRMAN)/shaders
VPATHRELEASE		= $(DEPTH)/bin/$(VBIN)/release/$(LIQRMAN)/shaders

DEBUGS 			= $(patsubst src/%,$(VPATHDEBUG)/%, $(SLOSHADERS))
RELEASES 		= $(patsubst src/%,$(VPATHRELEASE)/%, $(SLOSHADERS))
TARGETS 		= $(patsubst src/%,$(VPATHSHADERS)/%, $(SLOSHADERS))



#BIN_VERSION=debug

.SUFFIXES :
.SUFFIXES : $(SLOEXT) .sl 

all : debug

.sl.$(SLOEXT):
	$(CSL) $(SLFLAGS) $(INCLUDES) $<

debug : $(VPATHDEBUG) $(DEBUGS)
release : $(VPATHRELEASE) $(RELEASES)
newversion : $(VPATHSHADERS) $(TARGETS)

$(VPATHDEBUG) $(VPATHSHADERS) $(VPATHRELEASE) :
	mkdir -p $@

$(VPATHSHADERS)/%.$(SLOEXT) : src/%.sl
	@echo $@
	@$(CSL) $(SLFLAGS) $(INCLUDES) -o $@ $<

$(VPATHDEBUG)/%.$(SLOEXT) : src/%.sl
	@echo $@
	@$(CSL) $(SLFLAGS) $(INCLUDES) -o $@ $<

$(VPATHRELEASE)/%.$(SLOEXT) : src/%.sl
	@echo $@
	@$(CSL) $(SLFLAGS) $(INCLUDES) -o $@ $<


clean Clean :
	@rm -f $(SLOSHADERS) *.$(SLOEXT)

include $(DEPTH)/commonrules.mk
