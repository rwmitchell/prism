# "@(#)$Id$";
######################################################################
# You'll need both of these:
#    make.darwin
#    make.solaris

include make.$(OSTYPE)

CC_RELEASE_FLAGS = -O3
CC_DEBUG_FLAGS   = -g3 -DDEBUG_ALL

RLS  = release
DBG  = debug
PTH  = $(RLS)

.PHONY: release
release: CFLAGS += $(CC_RELEASE_FLAGS)
release: PTH    := $(RLS)
release: DSYM   := @echo
release: make_it

.PHONY: debug
debug: CFLAGS += $(CC_DEBUG_FLAGS)
debug: PTH    := $(DBG)
debug: DSYM   := dsymutil
debug: make_it

# Change "CPROG" and "SCRIPT" to something more appropriate
#
# Override this on the cmdline with: make prefix=/some/where/else
prefix = $(BLD)

DIR = $(shell basename $(CURDIR))
BLD = $(HOME)/Build
MCH = $(BLD)/$(MACHTYPE)
BAS = $(MCH)/$(PTH)/$(DIR)
DEP = $(MCH)/.dep/$(DIR)

DST = $(BAS)/bin
OBJ = $(BAS)/obj
LBJ = $(BAS)/lobj
LIB = $(BAS)/lib

SRC = Source
NST = $(prefix)/bin

# Additional object files used with other programs
LB_FILES = \
	$(OBJ)/strings.o  \
	$(OBJ)/io.o       \
	$(OBJ)/loadfile.o \
	$(OBJ)/helpd.o    \

SM_FILES = \
	$(OBJ)/shmem.o \
	$(OBJ)/now.o   \
	$(OBJ)/helpd.o \

SM_PROGS  = \
						$(DST)/traveler   \
						$(DST)/trvlr_send \

# Stand alone Programs
DST_PROGS = \
						$(DST)/asot_term     \
						$(DST)/getwinsz      \
						$(DST)/matrix        \
						$(DST)/iris          \

# All Scripts (basename, no extensions ie: foo, not foo.pl)
DST_SCRPT = \
#						$(DST)/color          \

DIRS = \
			$(BAS) \
			$(DEP) \
			$(DST) \
			$(NST)  \
			$(OBJ) \

$(DIRS):
	mkdir -p $@

$(DST)/%:	$(SRC)/%.pl
	install -m ugo+rx $< $@

$(DST)/%:	$(SRC)/%.sh
	install -m ugo+rx $< $@

$(NST)/%: $(DST)/%
	install -m ugo+rx $< $@

$(OBJ)/%.o	:	$(SRC)/%.c $(DEP)/%.d
	$(CC) -o $@ -c $(CFLAGS) $<
#	@echo "SRC DEPEND: $@ on $^"

######## Define C programs ###########

######## Identify what to Make #######

list:
	@echo "debug release"
	@echo "all install"
	@echo "foo"
	@echo "help help_install"

var:
	@ echo "OSTYPE  : $(OSTYPE)"
	@ echo "MACHTYPE: $(MACHTYPE)"
	@ echo "BASE    : $(BAS)"
	@ echo "BUILD   : $(BLD)"
	@ echo "DIRS    : $(DIRS)"
	@ echo
	@ echo "make all   to make the programs"

show_install:
	@echo ""
	@echo "These programs need to be installed:"
	@make -sn install

all_make: \
	$(DIRS)           \
	$(OBJ)            \
	$(DST)            \
	$(DST_PROGS)      \
	$(DST_SCRPT)      \
	$(SM_PROGS)       \
show_install        \


# 2007-08-22: does not compile
#	$(DST)/testbyteorder \

install:            \
	$(NST)/matrix     \
	$(NST)/traveler   \
	$(NST)/trvlr_send \

$(SM_PROGS):	$(DST)/% : $(OBJ)/%.o $(SM_FILES) $(LB_FILES)
	$(CC) -o $@ $^ $(LINKOPT)
	@echo
#	@echo "DEPF: $(DEPFILES)"
#	@echo "$@: $^"

$(DST_PROGS):	$(DST)/% : $(OBJ)/%.o $(DEPFILES) $(LB_FILES)
	$(CC) -o $@ $^ $(LINKOPT)
	$(DSYM) $@
	@echo
#	@echo "DEPF: $(DEPFILES)"
#	@echo "$@: $^"

##########################################################
# Dependency code added here
SUFFIXES += .d

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn install

#Find all the C files in the $(SRC)/ directory
SOURCES:=$(shell find $(SRC) -name "*.c")

#These are the dependency files, which make will clean up after it creates them
DEPFILES:=$(patsubst %.c,%.d,$(patsubst $(SRC)/%,$(DEP)/%, $(SOURCES)))

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPFILES)
endif

#This is the rule for creating the dependency files
$(DEP)/%.d: $(SRC)/%.c
	mkdir -p $(DEP)
	$(CC) $(CFLAGS) -I$(SRC) -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $< -MF $@

# End of - Dependency code added here

##########################################################

clean:
	$(RM) $(DEP)/*.d $(OBJ)/*.o $(DST_PROGS) $(DST_SCRPT) $(SM_PROGS)
	rmdir $(OBJ) $(DST)

######## Describe how to Install #####

$(NST)/bd:	$(DST)/bd
	$(INSTALL)

$(NST)/call:	$(DST)/call
	$(INSTALL)

$(NST)/weather:	$(DST)/weather
	$(INSTALL)

######## For CPROGS you'll need to describe the the .o file dependecies

# open_multiple.o:

misc.o:

make_it:
	@make PTH=$(PTH) CFLAGS="$(CFLAGS)" DSYM="$(DSYM)" all_make

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn install
#Find all the C++ files in the $(SRC)/ directory
SOURCES:=$(shell find $(SRC)  -name "*.c")
#These are the dependency files, which make will clean up after it creates them
DEPFILES:=$(patsubst %.c,%.d,$(patsubst $(SRC)/%,$(DEP)/%, $(SOURCES)))

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPFILES)
endif

#This is the rule for creating the dependency files
$(DEP)/%.d: $(SRC)/%.c $(DEP)
	@echo "START DEP: $@"
	@echo $(CC) $(CFLAGS) -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $<
	$(CC) $(CFLAGS) -MG -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $< > $@
	@echo "END   DEP: $@"
# End of - Dependency code added here

# Make a highlight file for types.  Requires Exuberant ctags and awk
types: $(SRC)/.types.vim
$(SRC)/.types.vim: $(SRC)/*.[ch]
	ctags --c-kinds=gstu -o- $(SRC)/*.[ch] |\
		awk 'BEGIN{printf("syntax keyword Type\t")}\
		{printf("%s ", $$1)}END{print ""}' > $@
	ctags --c-kinds=d -o- $(SRC)/*.h |\
		awk 'BEGIN{printf("syntax keyword Debug\t")}\
		{printf("%s ", $$1)}END{print ""}' >> $@
# End types

tags:
	ctags --fields=+l --langmap=c:.c.h Source/*

