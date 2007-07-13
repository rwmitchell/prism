# "@(#)$Id$";
######################################################################
# You'll need both of these:
#    make.darwin
#    make.solaris

include make.$(OSTYPE)

# Change "CPROG" and "SCRIPT" to something more appropriate

SRC = ./Source
MSC = ./misc
DST = ./bin
NST = /usr/local/p

%.o:  $(SRC)/%.c
	$(CC) -c $(CFLAGS) -I$(MSC) $<

%.o:  $(MSC)/%.c
	$(CC) -c $(CFLAGS) $<

######## Define C programs ###########

open_multiple_SRC= $(SRC)/open_multiple.c $(MSC)/io.c
open_multiple_OBJ= $(SRC)/open_multiple.o $(MSC)/io.o

######## Identify what to Make #######

all:                \
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

install:            \
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
	$(NST)/call       \


$(DST):
	mkdir $(DST)

######## Describe how to Make ########

$(DST)/open_multiple: $(open_multiple_OBJ)
	$(CC) $(CFLAGS) -o $@ $(open_multiple_OBJ) $(LIB)

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

######## Describe how to Install #####

# $(NST)/CPROG:	$(DST)/CPROG
#		$(INSTALL)

$(NST)/SCRIPT:	$(DST)/SCRIPT
	$(INSTALL)

######## For CPROGS you'll need to describe the the .o file dependecies

open_multiple.o:

misc.o:


