# "@(#)$Id$";
######################################################################
# You'll need both of these:
#    make.darwin
#    make.solaris

include make.$(OSTYPE)

# Change "CPROG" and "SCRIPT" to something more appropriate

SRC = ./Source
OBJ = ./obj
MSC = ./misc
DST = ./bin
NST = /usr/local/p

$(OBJ)/%.o:  $(SRC)/%.c
	$(CC) -c $(CFLAGS) -I$(MSC) -o $@ $<

$(OBJ)/%.o:  $(MSC)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

######## Define C programs ###########

open_multiple_SRC= $(SRC)/open_multiple.c $(MSC)/io.c
open_multiple_OBJ := $(open_multiple_SRC:%.c=%.o)
open_multiple_OBJ := $(open_multiple_OBJ:$(SRC)/%=$(OBJ)/%)

bd_SRC=	$(SRC)/bd.c
bd_OBJ :=	$(bd_SRC:%.c=%.o)
bd_OBJ := $(bd_OBJ:$(SRC)/%=$(OBJ)/%)

ifdef_SRC=	$(SRC)/ifdef.c
ifdef_OBJ :=	$(ifdef_SRC:%.c=%.o)
ifdef_OBJ := $(ifdef_OBJ:$(SRC)/%=$(OBJ)/%)

# this doesn't compile
tbo_SRC=	$(SRC)/testbyteorder.c
tbo_OBJ :=	$(tbo_SRC:%.c=%.o)
tbo_OBJ := $(tbo_OBJ:$(SRC)/%=$(OBJ)/%)

getopt_SRC=	$(SRC)/getopt_long.c
getopt_OBJ :=	$(getopt_SRC:%.c=%.o)
getopt_OBJ := $(getopt_OBJ:$(SRC)/%=$(OBJ)/%)

######## Identify what to Make #######

var:
	@ echo OSTYPE: $(OSTYPE)
	@ echo MACHTYPE: $(MACHTYPE)
	@ echo
	@ echo "make all   to make the programs"

all:                \
	$(OBJ)            \
	$(DST)            \
	$(DST)/color      \
	$(DST)/lockfile   \
	$(DST)/readline   \
	$(DST)/roman      \
	$(DST)/tk_canvas  \
	$(DST)/tk_frames  \
	$(DST)/tk_helloworld \
	$(DST)/tk_listbox \
	$(DST)/tk_windows \
	$(DST)/macperl    \
	$(DST)/maclist    \
	$(DST)/call       \
	$(DST)/open_multiple \
	$(DST)/bd         \
	$(DST)/ifdef      \
	$(DST)/getopt_long\
	$(DST)/weather    \
	$(DST)/smooth     \
	$(DST)/spline     \
	
# 2007-08-22: does not compile
#	$(DST)/testbyteorder \

install:            \
	$(NST)/call       \
	$(NST)/bd         \
	$(NST)/weather    \


dont_install:
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


$(OBJ):
	mkdir $(OBJ)

$(DST):
	mkdir $(DST)

######## Describe how to Make ########

$(DST)/open_multiple: $(open_multiple_OBJ)
	$(CC) $(CFLAGS) -o $@ $(open_multiple_OBJ) $(LIB)

$(DST)/bd: $(bd_OBJ)
	$(CC) $(CFLAGS) -o $@ $(bd_OBJ) $(LIB)

$(DST)/ifdef: $(ifdef_OBJ)
	$(CC) $(CFLAGS) -o $@ $(ifdef_OBJ) $(LIB)

$(DST)/getopt_long: $(getopt_OBJ)
	$(CC) $(CFLAGS) -o $@ $(getopt_OBJ) $(LIB)

$(DST)/testbyteorder: $(testbyteorder_OBJ)
	$(CC) $(CFLAGS) -o $@ $(testbyteorder_OBJ) $(LIB)

$(DST)/color:	$(SRC)/color.pl
	$(INSTALL)

$(DST)/lockfile:	$(SRC)/lockfile.pl
	$(INSTALL)

$(DST)/readline:	$(SRC)/readline.pl
	$(INSTALL)

$(DST)/roman:	$(SRC)/roman.pl
	$(INSTALL)

$(DST)/tk_canvas:	$(SRC)/tk_canvas.pl
	$(INSTALL)

$(DST)/tk_frames:	$(SRC)/tk_frames.pl
	$(INSTALL)

$(DST)/tk_helloworld:	$(SRC)/tk_helloworld.pl
	$(INSTALL)

$(DST)/tk_listbox:	$(SRC)/tk_listbox.pl
	$(INSTALL)

$(DST)/tk_windows:	$(SRC)/tk_windows.pl
	$(INSTALL)

$(DST)/macperl:	$(SRC)/macperl.pl
	$(INSTALL)

$(DST)/maclist:	$(SRC)/maclist.pl
	$(INSTALL)

$(DST)/call:	$(SRC)/call.pl
	$(INSTALL)

$(DST)/weather:	$(SRC)/weather.pl
	$(INSTALL)

$(DST)/smooth:	$(SRC)/smooth.pl
	$(INSTALL)

$(DST)/spline:	$(SRC)/spline.pl
	$(INSTALL)

######## Describe how to Install #####

$(NST)/bd:	$(DST)/bd
	$(INSTALL)

$(NST)/call:	$(DST)/call
	$(INSTALL)

$(NST)/weather:	$(DST)/weather
	$(INSTALL)

######## For CPROGS you'll need to describe the the .o file dependecies

open_multiple.o:

misc.o:

