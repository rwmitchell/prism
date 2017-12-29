#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // usleep()
#include <sys/ioctl.h>  // ioctl()
#include <sys/stat.h>   // fsize() / stat()
#include <fcntl.h>      // open()
#include <sys/types.h>  // read()
#include <sys/uio.h>    // read()
#include <unistd.h>     // read()
#include <termios.h>    // winsize
#include <string.h>     // strcpy()
#include <getopt.h>
#include <ctype.h>      // isdigit()
#include <stdbool.h>    // bool
#include <sys/time.h>   // gettimeofday()
#include <sys/param.h>  // str2arr(), INT_MAX, MIN()
#include "bugout.h"

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0,
     start =  0,
     end   = 60,
     nap   =  0;

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

int str2arr( char *mlstr, const char *FS, char ***arr, int lim ) {
  char *ch;
  int i, cnt, pos;

  if ( lim < 0 ) lim = INT_MAX;

  // count the number of FS in the string
  for (cnt=0, ch=mlstr; *ch != '\0' && cnt <= lim; ++ch )
    if ( strchr( FS, *ch ) != NULL ) cnt++;
  cnt++;

//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

  if ( cnt > lim ) cnt = lim;

  *arr = (char **) malloc( sizeof(char *) * cnt );   // alloc space for array of pointers

  ch = mlstr;
  for (i=0, cnt=0, pos=0; *ch != '\0' && cnt < lim-1; ++i, ++ch ) {
    if ( strchr( FS, *ch ) != NULL ) {
      *ch = '\0';
//    BUGOUT("%2d: >%s<\n", cnt, &mlstr[pos] );
      (*arr)[cnt++] = &mlstr[pos];
      pos = i+1;
    }
  }
  (*arr)[cnt++] = &mlstr[pos];   // save the last one
//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

#ifdef __APPLE__off
  BUGOUT("malloc_size(*arr) = %lu\n", malloc_size( *arr ) );
#endif

  return( cnt );
}
off_t fsize( char *name ) {
  off_t size = -1L;
  struct stat sbuf;

  if (stat(name,&sbuf) == 0)
    if ( (sbuf.st_mode & S_IFMT) == S_IFREG)
      size=sbuf.st_size;

  return(size);
}
char *loadfile( char *fname, off_t *f_sz ) {
  off_t   f_limit = 32 * 2<<19;            // ~ 32MB, arbitrary
  ssize_t rc;
  int     fd;
  static
  char *data = NULL;

  *f_sz = fsize( fname )+1;                // get file size
  if ( debug & 0x0001 )
    BUGOUT( "%12llu file size\n", *f_sz );

  if ( *f_sz > 0 && *f_sz < f_limit ) {    // is file size in bounds?
    data = (char *) malloc( *f_sz );       // allocate space for file
    memset(data, '\0', *f_sz );

    if ( debug & 0x0001 )
    BUGOUT( "%p data: bytes: %llu\n", data, *f_sz );

    if ( (fd=open( fname, O_RDONLY )) > 0 ) {
      rc = read( fd, data, *f_sz );        // put entire file into data
      close( fd );
    } else {
      BUGERR( "Unable to open %s, %d\n", fname, fd );
    }
  } else {
    BUGERR( "file size: %llu   limit = %llu\n", *f_sz, f_limit );  // size out of bounds
  }

  if ( debug & 0x0002 ) {
    BUGOUT( "File Contents\n" );
    STDOUT( "--------------\n" );
    STDOUT( "%s", data );
    STDOUT( "--------------\n" );
  }

  return( data );
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
  STDERR("-w %4d: seconds to pause at end\n", nap );
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
      rc,
      f_sz = 0,
      longindex;

  char *data,
       **arr;

  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":Xo:s:e:w:d:uh";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "example", no_argument,       NULL, 'X' },
    { "myopt",   optional_argument, NULL, 'o' },
    { "start",   required_argument, NULL, 's' },
    { "end",     required_argument, NULL, 'e' },
    { "wait",    required_argument, NULL, 'w' },
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
  scr_sz = w.ws_col * (w.ws_row-2);

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
      case 'w': nap   = strtol( optarg, NULL, 10 ); break;

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

  for (; optind < argc; optind++) {

    data = loadfile( argv[optind], (off_t *) &f_sz );
    rc   = str2arr( data, "\n", &arr, f_sz );

    memset(screen, ' ', scr_sz );

    int j;
    for ( i=0; i < MIN(rc, w.ws_row-1); ++i ) {
      strcpy( &screen[i*w.ws_col], arr[i] );
      for (j=0; j<w.ws_col; ++j ) {
        if ( screen[i*w.ws_col+j] == '\0' ) screen[i*w.ws_col+j] = ' ';
      }
    }
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

  int row, col,
      clr=0;    // color
  for ( i=0; i<scr_sz; ++i ) {
    clr++;
    clr %= 7;
    row = array[1][i] / w.ws_col;
    col = array[1][i] % w.ws_col;
    setpos( row+1, col+1 );
    printf("[00;%dm", clr+31);
    printf("%c", screen[row*w.ws_col + col]);
    printf("[m");
    fflush(stdout);
    usleep(500);
  }
  for ( i=0; i<scr_sz; ++i ) {
    row = array[2][i] / w.ws_col;
    col = array[2][i] % w.ws_col;
    setpos( row+1, col+1 );
//  printf("[00;%dm", clr+31);
    printf("%c", screen[row*w.ws_col + col]);
//  printf("[m");
    fflush(stdout);
    usleep(200);
  }

  if ( nap == 0 )
    setpos( sr-1, 0 );
  else
    setpos( sr-0, 0 );

  sleep(nap);

  exit(0);
}

