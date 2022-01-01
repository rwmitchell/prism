#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // usleep()
#include <sys/ioctl.h>  // ioctl()
#include <sys/types.h>  // read()
#include <sys/uio.h>    // read()
#include <termios.h>    // winsize
#include <string.h>     // strcpy()
#include <wchar.h>      // wclscpy()
#include <getopt.h>
#include <ctype.h>      // isdigit()
#include <stdbool.h>    // bool
#include <sys/time.h>   // gettimeofday()
#include <sys/param.h>  // str2arr(), INT_MAX, MIN()
#include <locale.h>
#include <wchar.h>
#include <mylib.h>
#include "traveler.h"

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024];   // temporary optarg value
int  debug =  0,
     lskp  =  1;

wint_t
     wch   =  0;

bool B_std   = true,
     B_shift = true,      // shift text
     B_time  = false,     // show time until next message
     B_rmid  = false;     // removed shmem

// basically copied from:
// https://stackoverflow.com/questions/6127503/shuffle-array-in-c
void init_rndgen() {
  static bool done = false;
  struct timeval tv;

  if ( !done ) {
    gettimeofday( &tv, NULL );
    int usec = tv.tv_usec;
    srand48(usec);
    done = true;
  }
}
void shuffle( int *array, size_t n ) {
  init_rndgen();

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

int shftarr( int row, int col, wchar_t **arr ) {
  int r, c;
  wchar_t ch, *pch;
  static int x=0;

  if ( x>1) x = -2;
  else ++x;

  for ( r=0; r<row-1; ++r ) {
     ch = *arr[r];
    pch =  arr[r];
    for ( c=0; c<col-2; ++c, ++pch ) *pch = *(pch+1)+x;
    *(pch+0) = ch + x;
    *(pch+1) = '\0';
  }
  return(0);
}
int arr2nab( int row, int col, wchar_t **arr, wchar_t *nab, bool show, int cnt, wchar_t **msg ) {
  int r,  c,
      ln, lb,
      mi = 0,
      rds, rdc,      // random style, color
      stl, clr;
  wchar_t *pch,
          *pnb;
  wchar_t nrml[16],
          bold[16];

  ln = swprintf(nrml, 16, L"[%d;%dm", 0, 1+31);  // normal  green
  lb = swprintf(bold, 16, L"[%d;%dm", 1, 1+31);  // bold    green

  if ( lb < 0 ) BUGOUT( "swprintf returned: %d\n", lb );

  pnb = nab;

  for ( r=0; r<row-1; ++r ) {

    if ( show && mi<cnt && r == 10+(mi*lskp)) pch = msg[mi++];
    else                                      pch = arr[r];

    for ( c=0; c<col-1; ++c ) {
      rds = ( (int) (drand48()*20 ) );  // gen random to select bold/normal
      rdc = ( (int) (drand48()*20 ) );  // gen random to select color

      stl = (rds%20 == 0);
      clr = (rdc%3  == 0);

      lb = swprintf(bold, 16, L"[%d;%dm", stl, 31+clr);  // bold    green

      if (   stl||clr ) { memcpy( (char *) pnb, bold, lb*4); pnb += lb; }
      *pnb++ = *pch++;
      if (   stl||clr )  { memcpy( (char *) pnb, nrml, ln*4); pnb += ln; }
    }
    *pnb++ = ' ';
  }
  *pnb++ = '\0';
  return(0);
}
wchar_t *add_msg( int flag, int llen, int mlen, wchar_t *line, const char *msg, wchar_t *str ) {
  static
  int offset = -20;
  int i, j,
      spos;

  /*
   * flag: 0 - msg_cnt
   * llen: screen width
   * mlen: msg length
   * line: random text line
   * msg : secret message
   * str : output string
   */

  spos = (llen-mlen)/2 + offset;
  for( i=0, j=0; i<llen; ++i ) {
    str[i] = line[i];
    if ( i >= spos & j<mlen) str[i] = msg[j++];
  }
  str[llen] = '\0';

  if ( !flag ) ++offset;    // only shift after one complete set of msg lines

  return(str);
}
wchar_t *dslv_msg( int cnt, int mi, int llen, int mcnt, wchar_t *line, const char *msg, wchar_t *str ) {
  static bool B_init = true;    // false disables shfl code
  static int **shfl;
  int i, j,
      spos,
      mlen;

  /*
   * cnt  is the column number
   * llen is the width the window
   * mcnt is the number of message lines
   * line is the current line of random text
   * msg  is the current msg line
   * str  is the new output
   *
   * we need to malloc space for llen * number of msg lines, which this doesn't know
   *
   * the goal is to:
   * - shuffle an array the size of the secret message
   * - on each call, copy the next n-shuffled chars of msg to output
   *   to allow the secret message to dissolve onto the screen
   */

  if ( B_init ) {         // generate shuffled list of numbers for each msg line

    B_init = false;
    shfl  = (int **) malloc( sizeof(int *) * mcnt );
    for( j=0; j<mcnt; ++j ) {
      shfl[j] = (int *) malloc( sizeof(int) * llen + 1 );
      for( i=0; i<llen; ++i ) shfl[j][i] = i;
      shuffle( shfl[j], llen );
    }
//  printf(""); fflush(stdout);
  }

  mlen = strlen( msg );

  spos = (llen-mlen)/2;                            // compute starting position
  for( i=0, j=0; i<llen; ++i ) {                   // loop through each letter in line
    str[i] = line[i];                              // copy input to output
    if ( i >= spos & j<mlen) str[i] = msg[j++];    // overwrite output with msg
  }
  str[llen] = '\0';

  cnt = (cnt*2>llen) ? llen : cnt*2;
  for( i=cnt; i<llen; ++i ) str[ shfl[mi][i] ] = line[ shfl[mi][i] ]; // rewrite output with input

  return(str);
}
int rnd2arr( int row, int col, wchar_t wch, wchar_t ***arr) {
  int r, c;
  wchar_t ch,
         *big = NULL, *pbg = NULL;

  row -= 1;

  big  = (wchar_t *)  RMmalloc( big,  sizeof( wchar_t * ) * (row*col+1));
  *arr = (wchar_t **) RMmalloc( *arr, sizeof( wchar_t * ) * row );   // alloc space for array of pointers

  init_rndgen();

  pbg  = big;
  for( r=0; r<row; ++r ) {

    (*arr)[r] = (wchar_t *) pbg;                           // assign arr to start of row

    for ( c=0; c<col-1; ++c ) {
#define RANDOM
#ifdef  RANDOM
      ch = (wchar_t) ( (unsigned int) (drand48()*40 + ' ') + wch );  // gen random chars
#else
      ch = 'F';
#endif
      *pbg = ch;
      ++pbg;
//    fputc( ch, stdout );
    }
//  fputc( '\n', stdout );

    *pbg = '\0';
    ++pbg;
  }
//printf("%ls", big );

  return(0);
}
#ifdef  NO_loadfile
int str2arr( char *mlstr, const char *FS, char ***arr, int lim ) {
  char    *ch;
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
      (*arr)[cnt++] = (char *) &mlstr[pos];
      pos = i+1;
    }
  }
  (*arr)[cnt++] = (char *) &mlstr[pos];   // save the last one
//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

#ifdef __APPLE__off
  BUGOUT("malloc_size(*arr) = %lu\n", malloc_size( *arr ) );
#endif

  return( cnt );
}
#endif

void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s]\n", progname, opt);
  STDERR("  -W %4X: wide characters\n", wch );
  STDERR("  -d INTEGER    [%d]\n", debug );
  STDERR("\n");
  STDERR("Get messages from the future\n");
  STDERR("\n");

  if ( debug ) RMhelpd( lopts );

  exit(-0);
}

int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt,
      i, j,
//    rc   = 0,
      longindex;

  wchar_t **arr = NULL,
           *nab = NULL;          // normal and bold text

  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":mRstW:d:uh";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "manual",  no_argument,       NULL, 'm' },
    { "reset",   no_argument,       NULL, 'R' },
    { "still",   no_argument,       NULL, 's' },
    { "time",    no_argument,       NULL, 't' },
    { "wide",    optional_argument, NULL, 'W' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { NULL,      0,                 NULL,  0  }
  };

  int scr_sz,
     *array[5],
      sr, sc;    // start row, column
  wchar_t *screen;

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);    // Get window size
  scr_sz = w.ws_col * (w.ws_row-2);

  getpos( &sr, &sc );

  screen = (wchar_t *) malloc( sizeof(wchar_t) * scr_sz );
  memset(screen, ' ', scr_sz );

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
          if ( argv[optind] == NULL ) {
            B_have_arg = false;
          } else {
            strcpy(myarg, argv[optind++]);
          }
        } else if ( *optarg == '-' ) {  // optarg is actually the next option
          --optind;
          B_have_arg = false;
        } else {

          strcpy(myarg, optarg);
        }
        break;
    }

    // Normal Check
    switch (opt) {
      case ':':              // check optopt for previous option
        switch( optopt ) {
          case 'W': wch = 0x400; break;
          case 'd': debug += dinc; BUGOUT("debug level: %d\n", debug ); dinc <<= 1; break;
          default : BUGOUT("No arg for %c\n", optopt ); break;
        }
        break;

      case 'm': B_std    = !B_std;   break;
      case 'R': B_rmid   = !B_rmid ; break;
      case 's': B_shift  = !B_shift; break;
      case 't': B_time   = !B_time ; break;

      case 'W': if ( B_have_arg )      // -wide
                     wch = strtol(myarg, NULL, 16 );
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

  // ***** Setup SHMEM ******
  bool shm_exists = false;
  int  shmid_secret;
  TRAVELER_t *block;
  size_t      block_sz;
  char        data[MAX_SECRET],
            **msg;
  int         msg_cnt;

  shm_exists = RMcheck_shmem( KEY_TRAVELER,  &shmid_secret );
//printf("SHMEM %s: %d\n", shm_exists ? "Exists" : "------", shmid_secret);

  block_sz = sizeof( TRAVELER_t );
  block    = (TRAVELER_t *) RMsetup_shmem( !shm_exists, KEY_TRAVELER, block_sz, &shmid_secret);

  if ( B_rmid ) {
    BUGOUT( "Releasing shared memory");
    shmctl(shmid_secret, IPC_RMID, (struct shmid_ds *) block);
    STDOUT( " - shared memory has been released\n");
    exit(0);
  }

  if ( B_time ) {
    if ( block->time > 1.0 ) STDOUT("%d\n", (int)(block->time-RMNOW()));
    exit(0);
  }

  if ( B_std && block->time <= 1.0 ) exit(0);

  STDOUT("....waiting for incoming msg....\n");
  if ( B_std && block->time > RMNOW() ) sleep( (int) (block->time - RMNOW()) );

  memcpy(data, block->text, MAX_SECRET );               // copy shmem to data
  msg_cnt = RMstr2arr( data, "\n", &msg, MAX_SECRET );    // split data into arrays
  lskp = block->lskp;

  STDOUT("Incoming: %d\n", msg_cnt )
//for( i=0; i<msg_cnt; ++i ) STDOUT("%s\n",  msg[i] );
  sleep( 5 );
  // ***** Setup SHMEM end **

  setlocale(LC_ALL, "" );

  // This allows us to use charset to find an interesting starting value,
  // and then adjust it for printable characters
  if ( wch > ' ' ) wch -= ' ';

  printf("]1337;HighlightCursorLine=no"); // Disable cursor guide in iTerm
  printf("]1337;CursorShape=1");          // set vertical cursor

  // Create a dummy test string
  for ( i=0; i<scr_sz; ++i ) screen[i] = ' ' + i%0x40 + wch;

//rc = w.ws_row;
  rnd2arr(w.ws_row, w.ws_col, wch, &arr );

  wchar_t *foo[32],
          *rndmsg;
  bool show=false;
  int start, stop,
      mi;
  start = w.ws_col * .25;
  stop  = start * 3;

  nab  = (wchar_t *)  RMmalloc( nab, sizeof( wchar_t *) * (w.ws_row*w.ws_col*17+1));

  // allocate space for all the msgs
  rndmsg = (wchar_t *) malloc( sizeof(wchar_t) * w.ws_col * msg_cnt + 1);
  for ( mi=0; mi<msg_cnt; ++mi )
    foo[mi] = &rndmsg[mi * w.ws_col];

  for ( j=0; j<w.ws_col-1; ++j ) {       // loop on columns
    setpos( 1, 1 );                      // reset cursor to home

    // Generate lines for output
    if ( B_shift ) {                     // defaults to true, shift text to left
      if ( j>start && j<stop && j%4) {   // show secret msg
        for ( mi=0; mi<msg_cnt; ++mi )   // loop on msg lines
          add_msg ( mi, w.ws_col-1, strlen(msg[mi]), arr[10+(mi*lskp)], msg[mi], foo[mi] );
        show=true;                       // show special lines with msg

      } else show = false;               // or not
    } else {                             // don't shift text
      if ( j>start && j<stop ) {         // show secret msg
        for ( mi=0; mi<msg_cnt; ++mi )   // loop on msg lines
//        dslv_msg( stop-start, w.ws_col-1, msg_cnt, arr[10+(mi*lskp)], msg[mi], foo[mi] );
          dslv_msg( j, mi, w.ws_col-1, msg_cnt, arr[10+(mi*lskp)], msg[mi], foo[mi] );
        show=true;                       // show special lines with msg

      } else show = false;
    }

    printf("[%d;%dm", 0, 1+31);        // set text to normal, green
#define NORMAL
#ifdef  NORMAL
#define PLAIN_no
#ifdef  PLAIN
    // display output
    for( i=0, mi=0; i<w.ws_row-1; ++i ) {   // show plain text
      if ( show && mi<msg_cnt && i == 10+(mi*lskp)) printf("%ls\n", foo[mi++] );
      else                                          printf("%ls\n", arr[i]    );
    }
#else                                       // generate Normal/Bold in single string
    arr2nab( w.ws_row, w.ws_col, arr, nab, show, msg_cnt, foo );
    printf("%ls\n", nab );
    fflush(stdout);
#endif
#else
    int st;
    for( i=0, mi=0; i<w.ws_row-1; ++i ) {
      st = ( (int) (drand48()*16 ) );       // gen random bold/normal
      if ( st%3 ) printf("[%d;%dm", 0, 1+31);
      else       printf("[%d;%dm", 1, 1+31);
      if ( show && mi<msg_cnt && i == 10+(mi*lskp)) {
        printf("%ls\n", foo[mi++] );
      } else {
        printf("%ls\n", arr[i]    );
      }
    }
#endif
    printf("[%d;%dm", 1, 1+31);        // reset text to bold, green


    if ( B_shift ) shftarr( w.ws_row, w.ws_col, arr );

    fflush(stdout);
    usleep( 300000 );
  }

  printf("[m");

  setpos( sr-0, 0 );
  printf("]1337;HighlightCursorLine=yes"); // enable cursor guide in iTerm
  printf("]1337;CursorShape=0");           // set block cursor

  block->time = 0.0;

  exit(0);
}

