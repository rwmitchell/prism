/* iris.c                                        2018-12-31  rwm
 * As a goddess, Iris is associated with communication, messages,
 * the rainbow and new endeavors. This personification of a rainbow
 * was once described as being a link to the heavens and earth.
 * In some texts she is depicted wearing a coat of many colors.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strcpy()
#include <getopt.h>
#include <ctype.h>   // isdigit()
#include <stdbool.h> // bool
#include "loadfile.h"
#include "bugout.h"
#include "helpd.h"

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0,
     ccnt  =  2;    // continuous colors

bool B_o   = false,
     B_256 = true,
     B_row = false,
     B_wrd = false;

enum {
  MAXCOLOR          =    6,
  IRIS_LIGHTMAGENTA = 0x8b008b,
  IRIS_PINK         = 0xffc0cb,
  IRIS_PINK2        = 0xeea9b8,
  IRIS_PINKNEON     = 0xff8bff,
  IRIS_LIGHTRED     = 0x8b0000,
  IRIS_LIGHTGREEN   = 0x90ee90,
  IRIS_LIGHTCYAN    = 0xe0ffff,
  IRIS_LIGHTBLUE    = 0xadd8e6,
  IRIS_LIGHTGREY    = 0xd3d3d3,
  IRIS_DARKGREEN    = 0x006400,
  IRIS_DARKGREY     = 0xa9a9a9,
  IRIS_RED          = 0xff0000,
  IRIS_GREEN        = 0x00ff00,
  IRIS_GREENALT     = 0x39a52f,
  IRIS_CYAN         = 0x00ffff,
  IRIS_CYANNEON     = 0x76ffff,
  IRIS_BLUE         = 0x0000ff,
  IRIS_PURPLE       = 0xa020f0,
  IRIS_LILAC        = 0x7080fb,
  IRIS_YELLOW       = 0xffff00,
  IRIS_ORANGE       = 0xffa500,
  IRIS_ORANGENEON   = 0xfd7674,
};
int rainbow[] = {
      IRIS_PINKNEON,
      IRIS_ORANGENEON,
      IRIS_YELLOW,
      IRIS_LIGHTGREEN,
      IRIS_CYANNEON,
      IRIS_LILAC,
//    IRIS_LIGHTBLUE
    },
    metal[] = {
      IRIS_LIGHTBLUE,
      IRIS_BLUE,
      IRIS_LIGHTGREY,
      IRIS_DARKGREY
    },
    greenbar[] = {
      IRIS_LIGHTGREEN,
      IRIS_GREEN,
      IRIS_GREENALT,
      IRIS_DARKGREEN,
    };

void set_cursor( bool on) {
  if ( on ) {
    printf("]1337;HighlightCursorLine=yes"); // enable cursor guide in iTerm
    printf("]1337;CursorShape=0");           // set block cursor
    printf("[%d;m", 0 );  // not bold
  } else {
    printf("]1337;HighlightCursorLine=no"); // Disable cursor guide in iTerm
    printf("]1337;CursorShape=1");          // set vertical cursor
    printf("[%d;m", 1 );  // bold
  }
}

void set_color8( short stl, short clr) {
  printf("[%d;%dm", stl, clr+31);
}

void set_color256( unsigned long clr) {
//printf("[%d;%dm", stl, clr+31);

  int R = (clr & 0xFF0000) >> 16,
      G = (clr & 0x00FF00) >>  8,
      B = (clr & 0x0000FF);

  printf(  "[38;2;%03d;%03d;%03dm", R, G, B);

//printf(  "[38;2;%02X;%02X;%02Xm", R, G, B);
//printf(  "[38;2;%02X;%02X;%02Xm", R, G, B);
//printf(" %06lX    %02x;%02x;%02xm", clr, R, G, B);
}

void inc_byrow( char ch, short *val, unsigned short cycle, int max ) {
  static short cnt = 0;

  if ( *val == -1 ) *val = cnt = 0;
  if ( cnt == cycle ) { (*val)++; *val %= max; }
  cnt %= cycle;
  if ( ch == '\n' ) ++cnt;
}
void inc_bycol( short *val, unsigned short cycle, int max ) {
  static short cnt = 0;

  if ( *val == -1 ) *val = cnt = 0;
  if ( cnt == cycle ) (*val)++;
   cnt %= cycle;
  ++cnt;
  *val %= max;
}
void inc_bywrd( char ch, short *val, unsigned short cycle, int max ) {
  static
  char och = '\0';
  static short cnt = 0;

  if ( cnt == cycle ) { (*val)++; *val %= max; }
  cnt %= cycle;
  if ( !isspace( och ) &&  isspace( ch ) ) cnt++;
  och = ch;

}
void one_line( const char *progname ) {
  STDOUT("%-20s: Colorize input\n", progname );
  exit(0);
}

void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("colorize text either by character column\n");
  STDERR("\n");
  STDERR("  -c CNT: change color every %d units\n", ccnt );
  STDERR("  -8: 8 bit    colors\n");
  STDERR("  -b: greenbar colors\n");
  STDERR("  -g: rainbow  colors\n");
  STDERR("  -m: metal    colors\n");
  STDERR("  -r: change color by row\n");
  STDERR("  -w: change color by word\n");
  STDERR("  -d INTEGER    (%d)\n", debug );
  STDERR("\n");

  if ( debug ) helpd( lopts );

  exit(-0);
}

int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt,
      longindex=0;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":o:c:8bgmrwd:uh1";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "myopt",   optional_argument, NULL, 'o' },
    { "cnt",     required_argument, NULL, 'c' },
    { "8bit",    no_argument,       NULL, '8' },
    { "bar",     no_argument,       NULL, 'b' },
    { "gay",     no_argument,       NULL, 'g' },
    { "metal",   no_argument,       NULL, 'm' },
    { "row",     no_argument,       NULL, 'r' },
    { "word",    no_argument,       NULL, 'w' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { "oneline", no_argument,       NULL, '1' },
    { NULL,      0,                 NULL,  0  }
  };

  int *palette = rainbow,
       sz_pal  = sizeof( rainbow ) / 4;

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

      case 'c': ccnt = strtol( optarg, NULL, 10 );
                BUGOUT( "%2d ccnt\n", ccnt );
                break;

      case '8': B_256 = !B_256;
                sz_pal = MAXCOLOR;
                break;
      case 'b': palette = greenbar;
                sz_pal  = sizeof( greenbar ) / 4;
                break;
      case 'g': palette = rainbow;
                sz_pal  = sizeof( rainbow ) / 4;
                break;
      case 'm': palette = metal;
                sz_pal  = sizeof( metal ) / 4;
                break;

      case 'r': B_row = !B_row; break;
      case 'w': B_wrd = !B_wrd; ccnt = 1; break;

      case 'o':
        B_o = !B_o;
        BUGOUT("B_have_arg = %s\n", TF[B_have_arg]);
        BUGOUT("myarg = %s\n", myarg);
        if ( myarg[0] != '\0' ) strcpy(myopt, myarg);
        BUGOUT("optional arg for (%s) is [%s]\n", longopts[longindex].name, myopt );
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
        usage( longopts );
        break;

      case '1': one_line( argv[0] ); break;

      case 'h':
      default :
        ++errflg;
        break;
    }
  }

  if (errflg) help(argv[0], opts, longopts);

  off_t  f_sz = 0;
  char *buf   = NULL,
       *pch;
  short clr = -1,    // color
        stl =  1;    // style

//BUGOUT("rainbow: %lu\n", sizeof( rainbow )/4);
//BUGOUT("  metal: %lu\n", sizeof(   metal )/4);

  set_cursor( false );
  if ( optind == argc ) buf = (char *) loadstdin( &f_sz );

  for (; f_sz || optind < argc; optind++) {         // process remainder of cmdline using argv[optind]

    if ( ! f_sz )
    buf   = (char *) loadfile ( argv[optind], &f_sz );

    pch = buf;
    while ( f_sz-- ) {
      if      ( B_row ) inc_byrow( *pch, &clr, ccnt, sz_pal );
      else if ( B_wrd ) inc_bywrd( *pch, &clr, ccnt, sz_pal );
      else              inc_bycol(       &clr, ccnt, sz_pal );

      if ( B_256 ) set_color256(      palette[clr] );
      else {
        set_color8  ( stl, clr );
        ++clr; clr %= 7;
//      ++stl; stl %= 7;
      }
      printf("%c", *pch );
      if ( !B_row && *pch == '\n' ) clr = -1;
      ++pch;
    }

    f_sz = 0;
    if ( buf  ) { free( buf  ); buf = NULL; }
  }                                         // for optind

  set_cursor( true  );
  exit(0);
}
