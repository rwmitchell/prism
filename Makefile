# "@(#)$Id$";
######################################################################
# You'll need both of these:
#    make.darwin
#    make.solaris

include make.$(OS)

RLS  = release
DBG  = debug
PTH := $(RLS)
RUN  = all

.PHONY: check
check: CFLAGS = $(CC_CHECK_FLAGS)
check: .analyze

.PHONY: check_all
check_all: CFLAGS = $(CC_CHECK_FLAGS)
check_all: make_check_all

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

# single column - 'column' has an input limit
scolumn = sed 's/ / 	/g' | tr -d '\n' | tr ' |' '\n\n'
columns = $(scolumn) | column -c 80

# Check if git exists
# If it does, get the current branch
# if it does and branch != main, add branch to DIR
# else leave DIR alone

GIT_VERSION := $(shell git --version 2>/dev/null)
$(warning GIT $(GIT_VERSION) )

GIT_VRSN := $(shell git --version | cut -f2 -d.)
$(warning GIT $(GIT_VRSN) )

DIR  = $(shell basename $(CURDIR))
ifdef GIT_VERSION
	BCH := $(shell git branch --show-current 2>/dev/null)
	BCH := $(if $(BCH),$(BCH),TEST)
#	BCH := $(shell echo main)
	ifeq ($(BCH),main)                   # Lack of whitespace is intentional
	else
		DIR     := $(DIR)-$(BCH)
	endif
endif

# Override this on the cmdline with: make buildroot=/some/where/else
# base for all compiler output
# buildroot = $(HOME)
buildroot = .

BLD = $(buildroot)/Build
INC = $(BLD)/include
MCH = $(BLD)/$(MACHTYPE)
BAS = $(MCH)/$(PTH)/$(DIR)
DEP = $(MCH)/.dep/$(DIR)

DST = $(BAS)/bin
OBJ = $(BAS)/obj
LBJ = $(BAS)/lobj
LIB = $(BAS)/lib

# Override this on the cmdline with: make prefix=/some/where/else
prefix = $(BLD)

SRC = Source
NST = $(prefix)/bin

MYINC = -I$(INC) -I$(SRC) -I$(HOME)/Build/include
MYLIB = -L$(BLD)/lib -L$(HOME)/Build/lib -lmylib $(LIBS)

DIRS =   \
	$(BLD) \
	$(DEP) \
	$(OBJ) \
	$(BAS) \
	$(DST) \
	$(NST)

# Optimistic selection
ALL_SRC  = $(wildcard $(SRC)/*.c)

# Realistic  selection
# ALL_SRC        \
#	$(SRC)/prism.c \

ALL_OBJ := $(ALL_SRC:%.c=%.o)
ALL_OBJ := $(ALL_OBJ:$(SRC)/%=$(OBJ)/%)

# Stand alone Programs
DST_PROGS =    \
	$(DST)/prism

$(DST_PROGS): $(ALL_OBJ)
	@ printf "\n"
	@ printf "Making: $@ $^" | $(scolumn)
	@ printf "\n\n"
	  $(CC) -o $@ $^ $(MYLIB)
	$(DSYM) $@
	@ printf "\n"

######## Identify what to Make #######

list:
	@echo "debug release"
	@echo "all install"
	@echo "help help_install"
	@echo $(DST_PROGS)

all:                \
	$(DIRS)           \
	$(ALL_OBJ)        \
	$(DST_PROGS)      \
	tags types        \
	help_install      \

$(OBJ)/%.o : $(SRC)/%.c $(DEP)/%.d
	@ echo "OBJ:" \
	$(CC) -o $@ -c $< $(CFLAGS) $(MYINC)
	$(CC) -o $@ -c $< $(CFLAGS) $(MYINC)

$(DIRS):
	mkdir -p $@

$(DST)/%:	$(SRC)/%.sh
	install -m ugo+rx $< $@

NST_PROGS = $(subst $(DST),$(NST),$(DST_PROGS))

$(NST)/%: $(DST)/%
	install -m ugo+rx $< $@

.PHONY: install help help_install

install:            \
	$(NST)            \
	$(NST_PROGS)      \

rebuild: clean all

reinstall: rebuild install

clean:
	@ printf "\n$(RM):\t"; printf "$(ALL_DEP)"    | $(scolumn); printf "\n"
	@ printf "\n$(RM):\t"; printf "$(ALL_OBJ)"    | $(scolumn); printf "\n"
	@ printf "\nrmdir:\t"; printf "$(DEP) $(OBJ)" | $(scolumn); printf "\n"
	$(RM) $(ALL_DEP) $(ALL_OBJ)
	rmdir $(DEP) $(OBJ) $(DST)

clobber: clean
	@ printf "\n$(RM):\t"; printf "$(DST_PROGS)"  | $(scolumn); printf "\n"
	@ printf "\nrmdir:\t"; printf "$(DST)"        | $(scolumn); printf "\n"
	$(RM) $(DST_PROGS) $(DST_SCRPT)
	rmdir $(DST)

vars:
	@ printf "Platform: %s - %s\n" $(OS) $(MACHTYPE)
	@ printf "\nOBJ :\t%s" $(OBJ)
	@ printf "\nDST :\t%s" $(DST)
	@ printf "\nNST :\t%s" $(NST)
	@ printf "\nSRC :\t%s" $(SRC)
	@ printf "\nSRC :\t"; printf "$(ALL_SRC)"   | $(scolumn); printf "\n"
	@ printf "\nDEP :\t"; printf "$(ALL_DEP)"   | $(scolumn); printf "\n"
	@ printf "\nOBJ :\t"; printf "$(ALL_OBJ)"   | $(scolumn); printf "\n"
	@ printf "\nDIR :\t"; printf "$(DIRS)"      | $(scolumn); printf "\n"
	@ printf "\nDST :\t"; printf "$(DST_PROGS)" | $(scolumn); printf "\n"
	@ printf "\nNST :\t"; printf "$(NST_PROGS)" | $(scolumn); printf "\n"

help: real_help help_install

real_help:
	@ printf "\nUse: '-buildroot=$(buildroot)' to specify build root"
	@ printf "\nUse: '-prefix=$(prefix)' to specify install root"
	@ printf "\nUse: 'make vars' : to show all paths\n"
	@ printf "\nmake release\n"
	@ printf "\nDST:\t"; printf "$(DST_PROGS)" | $(scolumn); printf "\n"
	@ printf "\nThese programs need to be compiled:\n"
	@ make -sn release

help_install:
	@ printf "\nmake install\n"
	@ printf "\nDST:\t"; printf "$(DST_PROGS)" | $(scolumn); printf "\n"
	@ printf "\nThese programs need to be installed:\n"
	@ make -sn install

.analyze: $(ALL_SRC)
	$(CC) $(CFLAGS) $?
	@ touch .analyze

make_check_all:
	@ $(RM) .analyze  || true
	@ make CFLAGS="$(CFLAGS)" check

make_it:
	@ echo "make_it: "$(CFLAGS)
	@ echo make PTH=$(PTH) CFLAGS="$(CFLAGS)" $(RUN)
	@ make PTH=$(PTH) CFLAGS="$(CFLAGS)" DSYM="$(DSYM)" $(RUN)


######################################

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn install

#These are the dependency files, which make will clean up after it creates them
ALL_DEP:=$(patsubst %.c,%.d,$(patsubst $(SRC)/%,$(DEP)/%, $(ALL_SRC)))

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(ALL_DEP)
endif

#This is the rule for creating the dependency files
$(DEP)/%.d: $(SRC)/%.c $(DEP)
	@echo "START DEP: $@"
	@echo $(CC) $(CFLAGS) -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $< | $(scolumn)
	$(CC) $(CFLAGS) -MG -MM -MT '$(patsubst $(SRC)/%,$(OBJ)/%, $(patsubst %.c,%.o,$<))' $(MYINC) $< > $@
	@echo "END   DEP: $@"
# End of - Dependency code added here

# Make a highlight file for types.  Requires Universal ctags and awk
CTAGS := $(shell ctags --version 2>/dev/null)

ifdef CTAGS
types: $(SRC)/.types.vim
tags: dotags

else
types: notypes
tags: nodotags
endif

nodotags:
	@echo No Tags to do

notypes:
	@echo No Types to do

$(SRC)/.types.vim: $(SRC)/*.[ch]
	ctags --kinds-c=gstu -o- \
		$(SRC)/*.[ch] \
		$(INC)/*.h \
		| grep -v "^__anon" \
		| awk 'BEGIN { printf("syntax keyword Type\t") } \
				{ printf("%s ", $$1) } END { print "" }' > $@
	ctags --kinds-c=d -o- \
		$(SRC)/*.h \
		$(INC)/*.h \
		| grep -v "^__anon" \
		| awk 'BEGIN{ printf("syntax keyword Debug\t") } \
				{ printf("%s ", $$1) } END { print "" }' >> $@
# End types

dotags: $(SRC)/*.[ch]
	ctags --fields=+l --langmap=c:.c.h \
		$(SRC)/* \
		$(INC)/*
