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
     B_wrd = false,
     B_test= false;

enum {
  MAXCOLOR          =    6,
};

// https://www.color-hex.com
const char *altcolors[] = {
   "#ff8bff#fd7674#ffff00#90ee90#76ffff#7080fb",  // rainbow
   "#90ee90#00ff00#39a52f#006400",                // greenbar
   "#add8e6#0000ff#d3d3d3#a9a9a9",                // metal?
   "#eeeeee#cccccc#bbbbbb#aaaaaa#777777",
   "#d0d8d9#c9d0ce#c6c8c9#bac4c8#b2babf",
   "#a70000#db1414#ec5300#f27038#ff8d00",
   "#9c2000#b44000#d86f05#f09d00#ffc100",
   "#00ff00#00e000#00c000#00a000#008000",
   "#1d945b#1b9f5f#1ba964#1ab268#17ba6b",
   "#72e736#15ff00#0fb200#69db93#54c372",
   "#6eff98#59d85e#38a44c#30864d#186038",
   "#ff005a#ffdc00#d9ff00#00ff1c#005cff",
   "#ff0000#ff8100#fff400#22ff00#0045ff",
   "#ff0000#ffff00#00ff00#0000ff#ff00ff",         // neon rainbow
   "#fe0000#fdfe02#0bff01#011efe#fe00f6",         // neon 0908
   "#244ca3#e56f1e#247a3b#d22626#568dd8",
   "#ccd8dc#b7c2c6#a3acb0#8c979a#7a8184",
   "#b87f0d#b07504#bd6700#b06304#a95b0e",
   "#d4582c#fb7125#ff861d#ff9529#fdab3a",
   "#7d0000#cd3700#cd7f32#ff8c00#ffa500",
   "#04daec#ea04ec#f9fb0a#01ff1f#5c01ee",
   "#8aff4f#3cff3f#2fff00#00ea33#00ba0d",
   "#fdff00#c9ff00#00ffdf#e302ff#ff00ce",
   "#03ff00#ff00bc#deff00#00cdff#8500ff",         // neon highlighter
   "#bf00ff#00ff7f#ff0000#007fff#ff00bf",
   "#156cf7#1d7af8#288af7#3399f8#41aef9",
   "#ffe700#fdff00#d6ff00#89ff00#2fff00",
};
void set_color256( unsigned long clr) {

  int R = (clr & 0xFF0000) >> 16,
      G = (clr & 0x00FF00) >>  8,
      B = (clr & 0x0000FF);

  printf(  "[38;2;%03d;%03d;%03dm", R, G, B);
}

void show_colors( ) {
  const
  char *pt;
  int i,
      cnt = sizeof( altcolors) / 8;
  unsigned long hex;

  BUGOUT("sizeof: %d\n", cnt );
  for (i=0; i<cnt; ++i ) {
    pt = altcolors[i];
    while ( ( pt=strchr( pt, '#') ) ) {
      pt++;
     hex = strtol( pt, NULL, 16 );
     set_color256(      hex );
      STDOUT("%2d: #%06lX ", i, hex ); fflush(stdout);
    }
    STDOUT("\n");
  }
  exit(0);
}
unsigned int get_colors( int ndx, unsigned int *hex ) {
  const
  char *pt;
  int pos = 0,
      cnt = sizeof( altcolors) / 8 - 1;

  if ( ndx < 0 || ndx > cnt ) {
    BUGERR("ndx %d is invalid, expected 0 to %d\n", ndx, cnt );
    exit( __LINE__ );
  }

  pt = altcolors[ndx];
  while ( ( pt=strchr( pt, '#') ) ) {
    pt++;
    hex[pos++] = strtol( pt, NULL, 16 );
  }

  return( pos );
}

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
  STDERR("  -p [0-%lu]: alternate palettes\n", sizeof( altcolors ) / 8 );
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
      pal_ndx  = 0 ,
      longindex= 0;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":o:c:8bgmp:rwtd:uh1";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "myopt",   optional_argument, NULL, 'o' },
    { "cnt",     required_argument, NULL, 'c' },
    { "8bit",    no_argument,       NULL, '8' },
    { "bar",     no_argument,       NULL, 'b' },
    { "gay",     no_argument,       NULL, 'g' },
    { "metal",   no_argument,       NULL, 'm' },
    { "palette", required_argument, NULL, 'p' },
    { "row",     no_argument,       NULL, 'r' },
    { "word",    no_argument,       NULL, 'w' },
    { "test",    no_argument,       NULL, 't' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { "oneline", no_argument,       NULL, '1' },
    { NULL,      0,                 NULL,  0  }
  };

  unsigned
  int palette[32] = { 0 },
      sz_pal = 0;

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
      case 'g': pal_ndx = 0; break;      // rainbow - gay
      case 'b': pal_ndx = 1; break;      // greenbar
      case 'm': pal_ndx = 2; break;      // metal

      case 'p': pal_ndx = strtol( optarg, NULL, 10 );
                break;

      case 'r': B_row  = !B_row;  break;
      case 'w': B_wrd  = !B_wrd;  ccnt = 1; break;
      case 't': B_test = !B_test; break;

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

  sz_pal = get_colors( pal_ndx, palette );

  if ( B_test ) show_colors();

  off_t  f_sz = 0;
  char *buf   = NULL,
       *pch;
  short clr = -1,    // color
        stl =  1;    // style

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
