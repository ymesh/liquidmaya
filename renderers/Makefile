# Common definition to compile shaders
DEPTH = ..
include $(DEPTH)/commondefs.mk


LGFILE := $(wildcard *.lg)

TARGETS = $(patsubst %.lg,$(VPATHRENDERS)/%.lg, $(LGFILE))


all :


newversion : $(VPATHRENDERS) $(TARGETS)

$(VPATHRENDERS) :
	mkdir -p $@


$(VPATHRENDERS)/%.lg : %.lg
	@echo $@
	@$(CP) $< $@

include $(DEPTH)/commonrules.mk
