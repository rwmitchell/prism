#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // usleep()
#include <sys/ioctl.h>  // ioctl()
#include <termios.h>    // winsize
#include <string.h>     // strcpy()
#include <getopt.h>
#include <ctype.h>      // isdigit()
#include <stdbool.h>    // bool
#include <sys/time.h>   // gettimeofday()
#include "bugout.h"

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0,
     start =  0,
     end   = 60;

bool B_o = false;

// basically copied from:
// https://stackoverflow.com/questions/6127503/shuffle-array-in-c
void shuffle( int *array, size_t n ) {
  struct timeval tv;
  gettimeofday( &tv, NULL );
  int usec = tv.tv_usec;
  srand48(usec);

  if ( n > 1 ) {
    size_t i;
    for (i = n-1; i>0; --i ) {
      size_t j = (unsigned int) (drand48()*(i+1));
      int    t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}
void setpos( int row, int col ) {
  printf("[%d;%dH", row, col);
}

bool getpos( int *row, int *col ) {
    fd_set readset;
    bool success = false;
    struct timeval time;
    struct termios term, initial_term;

    /*We store the actual properties of the input console and set it as:
    no buffered (~ICANON): avoid blocking 
    no echoing (~ECHO): do not display the result on the console*/
    tcgetattr(STDIN_FILENO, &initial_term);
    term = initial_term;
    term.c_lflag &=~ICANON;
    term.c_lflag &=~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    //We request position
    printf("[6n");
    fflush(stdout);

    //We wait 100ms for a terminal answer
    FD_ZERO(&readset);
    FD_SET(STDIN_FILENO, &readset);
    time.tv_sec = 0;
    time.tv_usec = 100000;

    //If it success we try to read the cursor value
    if (select(STDIN_FILENO + 1, &readset, NULL, NULL, &time) == 1) 
      if (scanf("\033[%d;%dR", row, col) == 2) success = true;

    //We set back the properties of the terminal
    tcsetattr(STDIN_FILENO, TCSADRAIN, &initial_term);

    return success;
}

void usage(struct option longopts[]) {
  int i;

  for (i=0; longopts[i].name != NULL; ++i ) {
    STDOUT("%s%c %c ",
      longopts[i].name,
      longopts[i].has_arg == optional_argument ? '=' : ' ',
      isprint(longopts[i].val) ? longopts[i].val : ' '
    );
  }
  STDOUT("\n");
  exit(0);
}
void help( char *progname, const char *opt, struct option lopts[] ) {
  int i;
  const char *args[] = {"None", "Required", "Optional"};

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("%s [-%s]\n", progname, opt);
  STDERR("-o=STRING     (%s : %s)\n", TF[B_o], myopt );
  STDERR("-s %4d: start value\n", start );
  STDERR("-e %4d: end   value\n", end   );
  STDERR("-d INTEGER    (%d)\n", debug );
  STDERR("try again later\n");

  if ( debug ) {
    STDERR("\n");
    STDERR("%2s %-15s %-9s %4s\n",
        "  ",
        "Name",
        "arg",
        "opt" );
    for (i=0; lopts[i].name != NULL; ++i ) {
      STDERR("%2d %-15s %-9s   %c",
        i,
        lopts[i].name,
        args[lopts[i].has_arg],
        isprint(lopts[i].val) ? lopts[i].val : ' '
      );
      if ( lopts[i].flag != NULL ) {
        STDERR(" Pointer: %d", *lopts[i].flag );
      }
      STDERR("\n");
    }
  }

  exit(-0);
}


int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt, i,
      longindex;

  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":Xo:s:e:d:uh";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "example", no_argument,       NULL, 'X' },
    { "myopt",   optional_argument, NULL, 'o' },
    { "start",   required_argument, NULL, 's' },
    { "end",     required_argument, NULL, 'e' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { NULL,      0,                 NULL,  0  }
  };

  int scr_sz,
     *array[5],
      sr, sc;    // start row, column
  char *screen;

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);    // Get window size
  scr_sz = w.ws_col * 20; // w.ws_row;

  getpos( &sr, &sc );

  screen = (char *) malloc( sizeof(char) * scr_sz );
  memset(screen, ' ', scr_sz );

  // Create a dummy test string
  for ( i=0; i<scr_sz; ++i ) screen[i] = '0' + i%10;
//setpos( 1, 0 );
//STDOUT("%s\n", screen);

  for ( i=0; i<5; ++i ) array[i] = (int *) malloc( sizeof(int) * scr_sz );

  strcpy(myopt, "defval");

  // parse command line options
  while ( ( opt=getopt_long_only(argc, argv, opts, longopts, &longindex )) != EOF ) {

    B_have_arg = true;
    memset( myarg, '\0', 1024 );      // reset

    if ( opt == 0 ) {                 // we got a longopt
      opt = longopts[longindex].val;  // set opt to short opt value
//  } else {
//    BUGOUT("shortopt: %c:%d (%s)\n", opt, opt, optarg );
    }

    // Pre-Check
    if ( optarg ) {                // only check if not null
      switch (opt) {               // check only args with possible STRING options
        case 'o':
        case 'd':
          if ( *optarg == '\0' ) {
            BUGOUT("optarg is empty\n");
            if ( argv[optind] == NULL ) {
              BUGOUT("next arg is also NULL\n");
              B_have_arg = false;
            } else {
              BUGOUT("next arg is %d:%s\n", optind, argv[optind] );
              strcpy(myarg, argv[optind++]);
            }
          } else if ( *optarg == '-' ) {  // optarg is actually the next option
            BUGOUT("optarg is: %s, probably next option\n", optarg);
            --optind;
            B_have_arg = false;
          } else {
            BUGOUT("optional arg for %s is %s\n", longopts[longindex].name, optarg );

            strcpy(myarg, optarg);
            BUGOUT("optarg = %c(%s)\n", *optarg, myarg);
          }
          break;
      }
    } else
      B_have_arg = false;          // optarg was null

    // Normal Check
    switch (opt) {
      case ':':              // check optopt for previous option
        BUGOUT("Got a Colon for: %c\n", optopt );
        B_have_arg = false;
        switch( optopt ) {
          case 'o': B_o = !B_o;    BUGOUT("No arg for o (%s)\n", myarg ); break;
          case 'd': debug += dinc; BUGOUT("debug level: %d\n", debug ); dinc <<= 1; break;
          default : BUGOUT("No arg for %c\n", optopt ); break;
        }
        break;

      case 'X':      // your code goes here
        break;

      case 'o':
        B_o = !B_o;
        BUGOUT("B_have_arg = %s\n", TF[B_have_arg]);
        BUGOUT("myarg = %s\n", myarg);
        if ( myarg[0] != '\0' ) strcpy(myopt, myarg);
        BUGOUT("optional arg for (%s) is [%s]\n", longopts[longindex].name, myopt );
        break;

      case 's': start = strtol( optarg, NULL, 10 ); break;
      case 'e': end   = strtol( optarg, NULL, 10 ); break;

      case 'd':                      // set debug level
        if ( B_have_arg ) {
          debug |= strtol(myarg, NULL, 16 );
          if ( debug == 0 ) {        // we didn't get a number
            debug = 1;               // we could verify by checking errno, but why?
            --optind;
          }
        } else {
          BUGOUT("increasing debug(%d) by %d\n", debug, dinc );
          debug += dinc; dinc <<= 1;
        }
        BUGOUT("debug level: 0x%02X : %02X\n", debug, dinc );
        break;

      case 'u': // output opts with spaces
        usage( longopts );
        break;

      case 'h':
      default :
        ++errflg;
        break;
    }
  }

  if (errflg) help(argv[0], opts, longopts);

  for (; optind < argc; optind++) {         // process remainder of cmdline using argv[optind]
    BUGOUT("%2d: %s\n", optind, argv[optind] );
  }                                         // for optind

  if ( start >= end ) {
    BUGERR("start (%d) must be less than end (%d), exiting\n", start, end );
    exit( -__LINE__ );
  }
  if ( end >= 2048 ) {
    BUGERR("end (%d) is beyond bounds (%d), exiting\n", end, 2048 );
  }

  for (i=0; i<5; ++i ) array[i] = (int *) malloc( sizeof(int) * scr_sz );

  for (i=0; i<scr_sz; ++i ) array[0][i] = i+start;

  int j;
  for (j=1; j<5; ++j ) {
    memcpy( array[j], array[j-1], sizeof(int) * scr_sz );
    shuffle( array[j], scr_sz );
  }

  int row, col;
  for ( i=0; i<scr_sz; ++i ) {
    row = array[1][i] / w.ws_col;
    col = array[1][i] % w.ws_col;
    setpos( row+2, col+1 );
    printf("%c", screen[row*w.ws_col + col]); fflush(stdout);
    usleep(5000);
  }

  setpos( 33, 0 );
  STDOUT("%s\n", screen);
  setpos( sr-1, 0 );

  exit(0);
}

