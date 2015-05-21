# "@(#)$Id$";
######################################################################
# You'll need both of these:
#    make.darwin
#    make.solaris

include make.$(OSTYPE)

CC_RELEASE_FLAGS = -O3
CC_DEBUG_FLAGS   = -g DDEBUG_ALL

.PHONY: debug
release: CCFLAGS += $(CC_RELEASE_FLAGS)
release: all_release

.PHONY: release
debug: CCFLAGS += $(CC_DEBUG_FLAGS)
debug: all_debug

# Change "CPROG" and "SCRIPT" to something more appropriate

DIR  = $(shell basename $(CURDIR))
BAS  = $(HOME)/Build/$(DIR)
BLD  = $(BAS)/$(MACHTYPE)
OBJR = $(BLD)/obj
OBJD = $(BLD)/objD
DSTR = $(BLD)/bin
DSTD = $(BLD)/binD

SRC  = Source
MSC  = misc
DEP  = $(BLD)/.dep
NST  = /usr/local/p

# Additional object files used with other programs
OBJR_FILES = \
						$(OBJR)/io.o

OBJD_FILES = \
						$(OBJD)/io.o

# All C programs
DSTR_PROGS = \
						$(DSTR)/open_multiple \
						$(DSTR)/bd \
						$(DSTR)/bittest \
						$(DSTR)/ifdef \

# All Scripts (basename, no extensions ie: foo, not foo.pl)
DST_SCRPT = \
						$(DSTR)/XXXSCRIPT1  \

DIRS = \
			$(BAS) \
			$(DEP) \
			$(DSTR) \
			$(DSTD) \
			$(NST)  \
			$(OBJR) \
			$(OBJD) \

$(DIRS):
	mkdir -p $@

$(DSTR)/%:	$(SRC)/%.pl
	install -m ugo+rx $< $@

$(DSTR)/%:	$(SRC)/%.sh
	install -m ugo+rx $< $@

$(NST)/%: $(DSTR)/%
	install -m ugo+rx $< $@

$(OBJR)/%.o:	$(SRC)/%.c $(DEP)/%.d
	$(CC) -c $(CFLAGS) -I$(SRC) -I$(MSC) -o $@ $<

$(OBJR)/%.o:	$(MSC)/%.c $(DEP)/%.d
	$(CC) -c $(CFLAGS)                   -o $@ $<

$(OBJD)/%.o:	$(SRC)/%.c $(DEP)/%.d
	$(CC) -c $(CFLAGS) -I$(SRC) -I$(MSC) -o $@ $<

$(OBJD)/%.o:	$(MSC)/%.c $(DEP)/%.d
	$(CC) -c $(CFLAGS)                   -o $@ $<

######## Define C programs ###########

# open_multiple_SRC= $(SRC)/open_multiple.c $(MSC)/io.c
# open_multiple_OBJ := $(open_multiple_SRC:%.c=%.o)
# open_multiple_OBJ := $(open_multiple_OBJ:$(SRC)/%=$(OBJR)/%)

# bd_SRC=	$(SRC)/bd.c
# bd_OBJ :=	$(bd_SRC:%.c=%.o)
# bd_OBJ := $(bd_OBJ:$(SRC)/%=$(OBJR)/%)

# bittest_SRC=	$(SRC)/bittest.c
# bittest_OBJ :=	$(bittest_SRC:%.c=%.o)
# bittest_OBJ := $(bittest_OBJ:$(SRC)/%=$(OBJR)/%)

ifdef_SRC=	$(SRC)/ifdef.c
ifdef_OBJ :=	$(ifdef_SRC:%.c=%.o)
ifdef_OBJ := $(ifdef_OBJ:$(SRC)/%=$(OBJR)/%)

# this doesn't compile
tbo_SRC=	$(SRC)/testbyteorder.c
tbo_OBJ :=	$(tbo_SRC:%.c=%.o)
tbo_OBJ := $(tbo_OBJ:$(SRC)/%=$(OBJR)/%)

getopt_SRC=	$(SRC)/getopt_long.c
getopt_OBJ :=	$(getopt_SRC:%.c=%.o)
getopt_OBJ := $(getopt_OBJ:$(SRC)/%=$(OBJR)/%)

######## Identify what to Make #######

list:
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

all_debug: \
	$(DIRS)            \
	$(OBJD)            \
	$(DSTD)            \
	$(DSTD_PROGS)      \


all_release: \
	$(DIRS)            \
	$(OBJR)            \
	$(DSTR)            \
	mylib              \
	$(DSTR_PROGS)      \

mylib:	\
	$(OBJR_FILES) \

#	$(DSTR)/color      \
#	$(DSTR)/lockfile   \
#	$(DSTR)/readline   \
#	$(DSTR)/open_multiple \
#	$(DSTR)/bd         \
#	$(DSTR)/ifdef      \
#	$(DSTR)/getopt_long\
#	$(DSTR)/bittest    \

# 2007-08-22: does not compile
#	$(DSTR)/testbyteorder \

install:            \
	$(NST)/bd         \
	$(NST)/bittest    \


dont_install:
	$(NST)/call       \
	$(NST)/color      \
	$(NST)/lockfile   \
	$(NST)/readline   \
	$(NST)/roman      \
	$(NST)/tk_canvas  \
	$(NST)/tk_frames  \
	$(NST)/tk_helloworld \
	$(NST)/tk_listbox \
	$(NST)/tk_windows \
	$(NST)/macperl    \
	$(NST)/maclist    \


$(DSTR_PROGS):	$(DSTR)/% : $(OBJR)/%.o $(OBJR_FILES) $(DEP)
	$(CC) -o $@ $< $(LINKOPT) $(OBJR_FILES) -L$(LIB)

##########################################################
# Dependency code added here
SUFFIXES += .d

#We don't need to clean up when we're making these targets
NODEPS:=clean tags svn install
#Find all the C++ files in the $(SRC)/ directory
SOURCES:=$(shell find $(SRC)/ -name "*.c")
#These are the dependency files, which make will clean up after it creates them
DEPFILES:=$(patsubst %.c,%.d,$(patsubst $(SRC)/%,$(DEP)/%, $(SOURCES)))

#Don't create dependencies when we're cleaning, for instance
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
    #Chances are, these files don't exist.  GMake will create them and
    #clean up automatically afterwards
    -include $(DEPFILES)
endif

#This is the rule for creating the dependency files
  # $(SRC)/%.h
#$(LDP)/%.d: $(LRC)/%.c
#	$(CC) $(CFLAGS) -I$(LRC) -MM -MT '$(patsubst $(LRC)/%,$(OBJR)/%, $(patsubst %.c,%.o,$<))' $< > $@

$(DEP)/%.d: $(SRC)/%.c $(DEP)
	$(CC) $(CFLAGS) -I$(LRC) -I$(SRC) -MM -MT '$(patsubst $(SRC)/%,$(OBJR)/%, $(patsubst %.c,%.o,$<))' $< > $@

$(DEP)/%.d: $(MSC)/%.c $(DEP)
	$(CC) $(CFLAGS) -I$(LRC) -I$(SRC) -MM -MT '$(patsubst $(MSC)/%,$(OBJR)/%, $(patsubst %.c,%.o,$<))' $< > $@

# End of - Dependency code added here
##########################################################

######## Describe how to Make ########

# $(DSTR)/open_multiple: $(open_multiple_OBJ)
# 	$(CC) $(CFLAGS) -o $@ $(open_multiple_OBJ) $(LIB)

#$(DSTR)/bd: $(bd_OBJ)
#	$(CC) $(CFLAGS) -o $@ $(bd_OBJ) $(LIB)

#$(DSTR)/bittest: $(bittest_OBJ)
#	$(CC) $(CFLAGS) -o $@ $(bittest_OBJ) $(LIB)

#$(DSTR)/ifdef: $(ifdef_OBJ)
#	$(CC) $(CFLAGS) -o $@ $(ifdef_OBJ) $(LIB)

$(DSTR)/getopt_long: $(getopt_OBJ)
	$(CC) $(CFLAGS) -o $@ $(getopt_OBJ) $(LIB)

$(DSTR)/testbyteorder: $(testbyteorder_OBJ)
	$(CC) $(CFLAGS) -o $@ $(testbyteorder_OBJ) $(LIB)

$(DSTR)/color:	$(SRC)/color.pl
	$(INSTALL)

$(DSTR)/lockfile:	$(SRC)/lockfile.pl
	$(INSTALL)

$(DSTR)/readline:	$(SRC)/readline.pl
	$(INSTALL)

$(DSTR)/roman:	$(SRC)/roman.pl
	$(INSTALL)

$(DSTR)/tk_canvas:	$(SRC)/tk_canvas.pl
	$(INSTALL)

$(DSTR)/tk_frames:	$(SRC)/tk_frames.pl
	$(INSTALL)

$(DSTR)/tk_helloworld:	$(SRC)/tk_helloworld.pl
	$(INSTALL)

$(DSTR)/tk_listbox:	$(SRC)/tk_listbox.pl
	$(INSTALL)

$(DSTR)/tk_windows:	$(SRC)/tk_windows.pl
	$(INSTALL)

$(DSTR)/macperl:	$(SRC)/macperl.pl
	$(INSTALL)

$(DSTR)/maclist:	$(SRC)/maclist.pl
	$(INSTALL)

$(DSTR)/call:	$(SRC)/call.pl
	$(INSTALL)

$(DSTR)/weather:	$(SRC)/weather.pl
	$(INSTALL)

$(DSTR)/smooth:	$(SRC)/smooth.pl
	$(INSTALL)

$(DSTR)/spline:	$(SRC)/spline.pl
	$(INSTALL)

######## Describe how to Install #####

$(NST)/bd:	$(DSTR)/bd
	$(INSTALL)

$(NST)/call:	$(DSTR)/call
	$(INSTALL)

$(NST)/weather:	$(DSTR)/weather
	$(INSTALL)

######## For CPROGS you'll need to describe the the .o file dependecies

open_multiple.o:

misc.o:

