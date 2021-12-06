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
#include <locale.h>
#include <wchar.h>
#include <mylib.h>

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024];   // temporary optarg value
int  debug =  0,
     nap   =  0,
     dly1  =  0,
     dly2  =  0,
     dly3  = 500000,
     speed = 10000;

wint_t
     wch   =  0;

bool B_o     = false,
     B_style = false,
     B_ctext = true;

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

#ifdef  NO_loadfile
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

  if ( ! file_exists( fname ) ) {
    BUGOUT("%s does not exist, exiting\n", fname );
    exit( __LINE__ );
  }

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
#endif
void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("  -c: toggle displaying clear text  [%s]\n", TF[B_ctext] );
  STDERR("  -p delay: set delay between pages [%d]\n", dly3/100000 );
  STDERR("  -s: toggle cycling text styles    [%s]\n", TF[B_style] );
  STDERR("  -S SPEED: set delay between chars [%d]\n", speed/1000 );
  STDERR("  -w %4d: seconds to pause at end\n", nap );
  STDERR("  -W %4X: wide characters\n", wch );
  STDERR("  -d INTEGER    [%d]\n", debug );
  STDERR("\n");
  STDERR("Reveal a screen of text by first writing an obfuscated\n");
  STDERR("version of the text, then rewrite with the clear text\n");
  STDERR("Get text to display from FILE or fill screen with\n");
  STDERR("printable characters\n");
  STDERR("\n");

  if ( debug ) RMhelpd( lopts );

  exit(-0);
}


int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt,
      i, j,
      rc   = 0,
      f_sz = 0,
      longindex;

  char *data = NULL,
       **arr = NULL;

  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":cp:sS:w:W:d:uh";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "clear",   no_argument,       NULL, 'c' },
    { "pause",   required_argument, NULL, 'p' },
    { "style",   no_argument,       NULL, 's' },
    { "speed",   required_argument, NULL, 'S' },
    { "wait",    required_argument, NULL, 'w' },
    { "wide",    optional_argument, NULL, 'W' },
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
  for ( i=0; i<scr_sz; ++i ) screen[i] = ' ' + i%0x40;

  for ( i=0; i<5; ++i ) array[i] = (int *) malloc( sizeof(int) * scr_sz );

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
    switch (opt) {               // check only args with possible STRING options
      case 'W':
      case 'd':
        if ( !optarg  ) {
//        BUGOUT("optarg is empty\n");
          if ( argv[optind] == NULL ) {
//          BUGOUT("next arg is also NULL\n");
            B_have_arg = false;
          } else {
//          BUGOUT("next arg is %d:%s\n", optind, argv[optind] );
            strcpy(myarg, argv[optind++]);
          }
        } else if ( *optarg == '-' ) {  // optarg is actually the next option
//        BUGOUT("optarg is: %s, probably next option\n", optarg);
          --optind;
          B_have_arg = false;
        } else {
//        BUGOUT("optional arg for %s is %s\n", longopts[longindex].name, optarg );

          strcpy(myarg, optarg);
//        BUGOUT("optarg = %c(%s)\n", *optarg, myarg);
        }
        break;
    }

    // Normal Check
    switch (opt) {
      case ':':              // check optopt for previous option
//      BUGOUT("Got a Colon for: %c\n", optopt );
        B_have_arg = false;
        switch( optopt ) {
          case 'o': B_o = !B_o;    BUGOUT("No arg for o (%s)\n", myarg ); break;
          case 'W': wch = 0x400; break;
          case 'd': debug += dinc; BUGOUT("debug level: %d\n", debug ); dinc <<= 1; break;
          default : BUGOUT("No arg for %c\n", optopt ); break;
        }
        break;

      case 'c': B_ctext = !B_ctext; break;
      case 's': B_style = !B_style; break;

      case 'p': dly3  = strtol( optarg, NULL, 10 );
                dly3  *=  100000;
                break;

      case 'S': speed = strtol( optarg, NULL, 10 );
                speed *= 1000;
                break;

      case 'w': nap   = strtol( optarg, NULL, 10 ); break;

      case 'W': if ( B_have_arg )      // -wide
                     wch = strtol(myarg, NULL, 16 );
//              BUGOUT("WCH: %0X %s : %s\n", wch, TF[B_have_arg], myarg )
//              sleep(5);
                if ( wch == 0 ) {      // did not get a number
                  --optind;
                  wch = 0x400;
                }
                break;

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

  setlocale(LC_ALL, "" );

  dly1    = speed / ( scr_sz / 100.0);
  dly2    = dly1 / 1.5;                      // speed up output a little

  // This allows us to use charset to find an interesting starting value,
  // and then adjust it for printable characters
  if ( wch > ' ' ) wch -= ' ';

  // ^[]1337;HighlightCursorLine=boolean^G
  printf("]1337;HighlightCursorLine=no"); // Disable cursor guide in iTerm
  printf("]1337;CursorShape=1");          // set vertical cursor

  if ( optind >= argc ) {
    setpos( 1, 0 );
    if ( wch > 0 ) {
      for ( i=0; i<scr_sz; ++i ) printf("%lc", screen[i] + wch );
      printf("[m");
    } else
      STDOUT("%s\n", screen);
  }

  for (; optind < argc; optind++) {

    if ( data ) {
      free( data);
      free( arr );
    }

    data = (char *) RMloadfile( argv[optind], (off_t *) &f_sz, false );
    rc   = RMstr2arr( data, "\n", &arr, f_sz );

    memset(screen, ' ', scr_sz );

#define FASTDEMO_no
#ifdef  FASTDEMO
    setpos( 1, 1 );
    printf("%s\n", screen); fflush(stdout);    // XYZZY
    setpos( 1, 1 );
    for(i=0; i<MIN(rc, w.ws_row-1); ++i) printf("%s\n", arr[i-0]);
    sleep(1);
    setpos( 1, 1 );
    printf("%s\n", screen); fflush(stdout);    // XYZZY
    setpos( 1, 1 );
    for(i=MIN(rc, w.ws_row-1); i>0; --i) printf("%s\n", arr[i-1]);

    sleep(1);
#endif

    for ( i=0; i < MIN(rc, w.ws_row-1); ++i ) {
      strcpy( &screen[i*w.ws_col], arr[i] );
      for (j=0; j<w.ws_col; ++j ) {
        if ( screen[i*w.ws_col+j] == '\0' ) screen[i*w.ws_col+j] = ' ';
      }
    }

    for (i=0; i<5; ++i ) array[i] = (int *) malloc( sizeof(int) * scr_sz );

    for (i=0; i<scr_sz; ++i ) array[0][i] = i;

    for (j=1; j<5; ++j ) {
      memcpy( array[j], array[j-1], sizeof(int) * scr_sz );
      shuffle( array[j], scr_sz );
    }

    wint_t ch;
    int row, col,
        clr=0,    // color
        stl=0;    // style
    for ( i=0; i<scr_sz; ++i ) {     // print text in random colors/symbols
      clr++; clr %= 7;

      if ( B_style ) { stl++; stl %= 7; }

      row = array[1][i] / w.ws_col;
      col = array[1][i] % w.ws_col;
      setpos( row+1, col+1 );
      printf("[%d;%dm", stl, clr+31);

      ch = screen[row*w.ws_col + col ];
      if ( ch != ' ' ) ch += wch;
      printf("%lc", ch );
      printf("[m");
      fflush(stdout);
      usleep(dly1);
    }
    usleep(dly3);

    printf("[01;%dm", 32);         // change text to normal;green

    if ( B_ctext )                   // optionally -v
    for ( i=0; i<scr_sz; ++i ) {     // print clear text
      row = array[2][i] / w.ws_col;
      col = array[2][i] % w.ws_col;
      setpos( row+1, col+1 );

      printf("%c", screen[row*w.ws_col + col]);

      fflush(stdout);
      usleep(dly2);
    }
    printf("[m");

    sleep(nap);
  }                                         // for optind


  if ( nap == 0 || argc > 1 )
    setpos( sr-1, 0 );
  else
    setpos( sr-0, 0 );
  printf("]1337;HighlightCursorLine=yes"); // enable cursor guide in iTerm
  printf("]1337;CursorShape=0");           // set block cursor

  exit(0);
}

