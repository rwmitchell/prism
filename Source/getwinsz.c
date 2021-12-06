// From:
// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>  // strcpy()
#include <getopt.h>
#include <ctype.h>   // isdigit()
#include <fcntl.h>   // open()
#include <stdbool.h> // bool
#include <mylib.h>

// Reports the size of the terminal if run directly,
// but cannot be run inside backticks or by another program


const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0;

bool B_o       = false,
     B_verbose = false,
     B_left    = false,
     B_right   = false,
     B_top     = false;

void setpos( int row, int col ) {
  printf("[%d;%dH", row, col);
}

// From: https://stackoverflow.com/questions/16026858/reading-the-device-status-report-ansi-escape-sequence-reply
bool getpos( int *row, int *col ) {
    fd_set readset;
    bool success = false;
    struct timeval time;
    struct termios term, initial_term;

    /* We store the actual properties of the input console and set it as:
       no buffered (~ICANON): avoid blocking
       no echoing (~ECHO): do not display the result on the console */
    tcgetattr(STDIN_FILENO, &initial_term);
    term = initial_term;
    term.c_lflag &=~ICANON;
    term.c_lflag &=~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    // We request position
    printf("[6n");
    fflush(stdout);

    // We wait 100ms for a terminal answer
    FD_ZERO(&readset);
    FD_SET(STDIN_FILENO, &readset);
    time.tv_sec = 0;
    time.tv_usec = 100000;

    // If it success we try to read the cursor value
    if (select(STDIN_FILENO + 1, &readset, NULL, NULL, &time) == 1)
      if (scanf("\033[%d;%dR", row, col) == 2) success = true;

    // We set back the properties of the terminal
    tcsetattr(STDIN_FILENO, TCSADRAIN, &initial_term);

    return success;
}

void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("%s [-%s]\n", progname, opt);
  STDERR("-o=STRING     (%s : %s)\n", TF[B_o], myopt );
  STDERR("-d INTEGER    (%d)\n", debug );
  STDERR("try again later\n");

  if ( debug ) RMhelpd( lopts );

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
  char *opts=":Xo:lrtvd:uh";     // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "example", no_argument,       NULL, 'X' },
    { "myopt",   optional_argument, NULL, 'o' },
    { "left",    no_argument,       NULL, 'l' },
    { "right",   no_argument,       NULL, 'r' },
    { "top",     no_argument,       NULL, 't' },
    { "verbose", no_argument,       NULL, 'v' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { NULL,      0,                 NULL,  0  }
  };

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);    // Get window size

  strcpy(myopt, "defval");

  // parse command line options
  while ( ( opt=getopt_long_only(argc, argv, opts, longopts, &longindex )) != EOF ) {

    B_have_arg = true;
    memset( myarg, '\0', 1024 );      // reset

    if ( opt == 0 ) {                 // we got a longopt
      opt = longopts[longindex].val;  // set opt to short opt value
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
#ifdef OLDBLOCK
        if ( !optarg ) {
          BUGOUT("optional arg for %s is NULL\n", longopts[longindex].name );
          BUGOUT("using previous value: %s\n", myarg );
          B_have_arg = false;
        } else {

          if ( *optarg == '\0' ) {
            BUGOUT("optarg is empty\n");
            BUGOUT("next arg is %d:%s\n", optind, argv[optind] );
            strcpy(myarg, argv[optind++]);
          } else {
            BUGOUT("setting myarg to: %s\n", optarg );
            strcpy(myarg, optarg);
          }
        }
#else
        BUGOUT("B_have_arg = %s\n", TF[B_have_arg]);
        BUGOUT("myarg = %s\n", myarg);
        if ( myarg[0] != '\0' ) strcpy(myopt, myarg);
#endif
        BUGOUT("optional arg for (%s) is [%s]\n", longopts[longindex].name, myopt );
        break;

      case 'l': B_left    = !B_left   ; break;
      case 'r': B_right   = !B_right  ; break;
      case 't': B_top     = !B_top    ; break;
      case 'v': B_verbose = !B_verbose; break;

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
        RMusage( longopts );
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

  int row, col;
  getpos( &row, &col);
//printf("Row: %d Col: %d\n", row, col );

  if ( B_verbose) {
    printf ( "lines %d\n", w.ws_row );
    printf ( "columns %d\n", w.ws_col );
  } else {
    if ( ! ( B_left || B_right || B_top) )
      printf( "%d %d\n", w.ws_row, w.ws_col );
    else {
      if ( B_top ) {
        for (i=10; i<w.ws_col; i+= 10 )
          printf("[%d;%dH%2d", 0, i, i/10 );
      }

      for ( i=1; i<w.ws_row; ++i ) {
        if ( B_left ) printf("[%d;%dH%2d", i, 0, i-1 );
        if ( B_right) printf("[%d;%dH%2d", i, w.ws_col-2, i-1 );
      }
//    setpos( w.ws_row-1, 0 );
      setpos( row-1, 0 );
    }
  }

  exit(0);
}
