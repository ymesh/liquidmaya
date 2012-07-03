# Common definition to compile shaders
DEPTH = ..
include $(DEPTH)/commondefs.mk


XPMICONS := $(wildcard *.xpm)
IFFICONS := $(wildcard *.iff)

TARGETS = $(patsubst %.xpm,$(VPATHICONS)/%.xpm, $(XPMICONS)) $(patsubst %.iff,$(VPATHICONS)/%.iff, $(IFFICONS))


all :


newversion : $(VPATHICONS) $(TARGETS)

$(VPATHICONS) :
	mkdir -p $@


$(VPATHICONS)/%.xpm : %.xpm
	@echo $@
	@$(CP) $< $@

$(VPATHICONS)/%.iff : %.iff
	@echo $@
	@$(CP) $< $@


include $(DEPTH)/commonrules.mk
