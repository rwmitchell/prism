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

.o:	$(SOURCE)/%.c
	$(CC) -c $(CFLAGS) -I$(MISC) $<

.o:	$(MSC)/%.c
	$(CC) -c $(CFLAGS) $<

######## Define C programs ###########

# CPROG_SRC = CPROG.c

# CPROG_OBJ = CPROG.o

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


$(DST):
	mkdir $(DST)

######## Describe how to Make ########

# $(DST)/CPROG: $(CPROG_OBJ
# 	$(CC) $(CFLAGS) -o $@ $(CPROG_OBJ) $(LIB)

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

######## Describe how to Install #####

# $(NST)/CPROG:	$(DST)/CPROG
#		$(INSTALL)

$(NST)/SCRIPT:	$(DST)/SCRIPT
	$(INSTALL)

######## For CPROGS you'll need to describe the the .o file dependecies

