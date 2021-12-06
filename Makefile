# "@(#)$Id$";
######################################################################
# You'll need both of these:
#    make.darwin
#    make.solaris

include make.$(OS)

CC_DEBUG_FLAGS  =-g3 -DDEBUG_ALL -DHGVERSION=\\\"\"${HGVERSIONf}\"\\\"
CC_RELEASE_FLAGS=                -DHGVERSION=\\\"\"${HGVERSIONf}\"\\\"

HGVERSION:= $(shell hg parents --template 'hgid: {date|date}')

UID:= $(shell id -u)

RLS  = release
DBG  = debug
PTH := $(RLS)
RUN  = all_make

column = sed 's/ / 	/g' | tr ' |' '\n\n'

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

MYINC = -I$(BLD)/include -I$(SRC)
MYLIB = -L$(BLD)/lib -lmylib

.PHONY: release
release: CFLAGS += $(CC_RELEASE_FLAGS) $(MYINC)
release: PTH    := $(RLS)
release: DSYM   := @echo
release: make_it

.PHONY: debug
debug: CFLAGS += $(CC_DEBUG_FLAGS) $(MYINC)
debug: PTH    := $(DBG)
debug: DSYM   := dsymutil
debug: make_it


SRC = Source
NST = $(prefix)/bin

# Stand alone Programs
DST_PROGS = \
						$(DST)/asot_term     \
						$(DST)/getwinsz      \
						$(DST)/matrix        \
						$(DST)/prism         \
						$(DST)/ttl           \
						$(DST)/traveler   \
						$(DST)/trvlr_send \

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
	@ echo "OBJ:" \
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -c $< -o $@
#	@echo "SRC DEPEND: $@ on $^"

######## Define C programs ###########

######## Identify what to Make #######

list:
	@echo "debug release"
	@echo "all install"
	@echo "foo"
	@echo "help help_install"

var:
	@ echo "Platform: " $(OS) " - " $(MACHTYPE)
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
show_install        \


# 2007-08-22: does not compile
#	$(DST)/testbyteorder \

install:            \
	$(NST)/matrix     \
	$(NST)/traveler   \
	$(NST)/trvlr_send \
	$(NST)/prism      \

$(DST_PROGS):	$(DST)/% : $(OBJ)/%.o $(DEPFILES)
	$(CC) -o $@ $^ $(MYLIB)
	$(DSYM) $@
	@echo
#	@echo "DEPF: $(DEPFILES)"
#	@echo "$@: $^"

clean:
	$(RM) $(DEP)/*.d $(OBJ)/*.o $(DST_PROGS) $(DST_SCRPT)
	rmdir $(OBJ) $(DST)

######## Describe how to Install #####

######## For CPROGS you'll need to describe the the .o file dependecies

# open_multiple.o:

misc.o:

make_it:
	@make PTH=$(PTH) CFLAGS="$(CFLAGS)" DSYM="$(DSYM)" all_make

# we jump through hoops with HGVERSIONf because chrooted vm1 lacks hg
stamp: hgstamp
	@ echo ${HGVERSIONf}

hgstamp: dummy
	@ [ -f $@ ] || touch $@
	@ echo $(HGVERSION) | cmp -s $@ - || echo $(HGVERSION) >$@

dummy: ;

HGVERSIONf:= $(shell cat hgstamp)

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
	@echo $(CC) $(CFLAGS) -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $< | $(column)
	$(CC) $(CFLAGS) -MG -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $(MYINC) $< > $@
	@echo "END   DEP: $@"
# End of - Dependency code added here

# Make a highlight file for types.  Requires Universal ctags and awk
types: $(SRC)/.types.vim
$(SRC)/.types.vim: $(SRC)/*.[ch]
	ctags --kinds-c=gstu -o- \
		$(SRC)/*.[ch] \
		$(BLD)/include/*.h \
		| grep -v "^__anon" \
		| awk 'BEGIN { printf("syntax keyword Type\t") } \
				{ printf("%s ", $$1) } END { print "" }' > $@
	ctags --kinds-c=d -o- \
		$(SRC)/*.h \
		$(BLD)/include/*.h \
		| grep -v "^__anon" \
		| awk 'BEGIN{ printf("syntax keyword Debug\t") } \
				{ printf("%s ", $$1) } END { print "" }' >> $@
# End types

tags: $(SRC)/*.[ch]
	ctags --fields=+l --langmap=c:.c.h \
		$(SRC)/* \
		$(BLD)/include/*
