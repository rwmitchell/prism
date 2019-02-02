/* iris.c                                        2018-12-31  rwm
 * As a goddess, Iris is associated with communication, messages,
 * the rainbow and new endeavors. This personification of a rainbow
 * was once described as being a link to the heavens and earth.
 * In some texts she is depicted wearing a coat of many colors.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    // strcpy()
#include <getopt.h>
#include <ctype.h>     // isdigit()
#include <stdbool.h>   // bool
#include <sys/param.h> // MAX()
#include "loadfile.h"
#include "bugout.h"
#include "helpd.h"

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0,
     ccnt  =  2;    // contiguous colors

enum { MAX_SEQ = 64 };
int SEQ[] = { [0 ... MAX_SEQ] = -1 },
      sz_seq = 0;

bool 
     B_256   = true,
     B_bkgnd = false,
     B_fix   = false,
     B_row   = false,
     B_wrd   = false,
     B_test  = false,
     B_tty   = false,
     B_brght = false;

#define MAX_CON 28.0

enum {
  MAXCOLOR =    6,
  FGC      =   38,         // ansi code to set foreground color
  BGC      =   48,         // ansi code to set background color
};

const char *foo[] = {
  "#77777F#777780#777781#888888#77778F",
  "#777685#777685#777685#777686#77768F",
  "#777685#777685#77758A#77758B#77768F",
  "#777685#777685#777490#777491#77768F",
  "red"
};

int  append_SEQ   ( int val ) {                      // append -y args to an array
  int cnt=0,
     *ps = SEQ;

  while ( cnt++ < MAX_SEQ && *ps != -1  ) ++ps;
  *ps = val;

  return( cnt );
}
short   set_SEQ   ( short nseq, int npal, unsigned int pal[] ) {   // set the color sequence order
  short i,
        t;

  if ( debug & 0x0002 ) {
    STDOUT("%2d: nseq\n", nseq );
    STDOUT("%2d: npal\n", npal );
  }

  for ( i=0; i<nseq; ++i ) {
    if ( SEQ[i] > npal ) {
      BUGOUT("%2d: index %2d too large, %2d is max\n", i, SEQ[i], npal );
      SEQ[i] %= npal;
    }
    SEQ[i]--;     // index starts at 0, decrement values
  }

  if ( nseq > 0 ) {
    t = SEQ[ nseq-1 ];
    t++;
  } else t = 0;

//for ( i=nseq; i<npal; ++i, ++t ) { STDOUT("%2d: %2d %2d\n", i, t, t%npal ); SEQ[i] = t%npal; }
  for ( i=nseq; i<npal; ++i, ++t ) SEQ[i] = t%npal;
  nseq = MAX( nseq, npal );
  for ( i=0; i<nseq; ++i ) SEQ[i] = pal[ SEQ[i] ];

  return( nseq );
}
void   list_SEQ   ( ) {
  int cnt=0,
     *ps = SEQ;

  STDOUT("Y: " );
  while ( cnt++ < MAX_SEQ && *ps != -1  ) STDOUT(" %06X", *ps++ );
  STDOUT("\n");
}
// https://www.color-hex.com
const char *altcolors[] = {
   "#ff8bff#fd7674#ffff00#90ee90#76ffff#7080fb",  // rainbow
   "#90ee90#00ff00#39a52f#006400",                // greenbar
   "#add8e6#d3d3d3#a9a9a9#a0a0ff",                // metal?
   "#a70000#db1414#ec5300#f27038#ff8d00",
   "#9c2000#b44000#d86f05#f09d00#ffc100",
   "#00ff00#00e000#00c000#00a000#008000",
   "#1d945b#1b9f5f#1ba964#1ab268#57ba6b",
   "#72e736#15ff00#0fb200#69db93#54c372",
   "#6eff98#59d85e#38a44c#30864d#388048",
   "#ff005a#ff8c00#d9ff00#22ff00#805cff",
   "#ff0000#ffff00#00ff00#7f7fff#ff00ff",         // neon rainbow
   "#fe0000#fdfe02#0bff01#8f8ffe#fe00f6",         // neon 0908
   "#244ca3#e56f1e#247a3b#d22626#568dd8",
   "#ccd8dc#b7c2c6#a3acb0#8c979a#7a8184",
   "#b87f0d#b07504#bd6700#b06304#a95b0e",
   "#d4582c#fb7125#ff861d#ff9529#fdab3a",
   "#bd0000#cd3700#cd7f32#ff8c00#ffa500",
   "#04daec#ea04ec#f9fb0a#01ff1f#5c01ee",
   "#8aff4f#3cff3f#2fff00#00ea33#00ba0d",
   "#fdff00#c9ff00#00ffdf#e302ff#ff00ce",
   "#03ff00#ff00bc#deff00#00cdff#8500ff",         // neon highlighter
   "#bf00ff#00ff7f#ff0000#007fff#ff00bf",
   "#156cf7#1d7af8#288af7#3399f8#41aef9",
   "#ffe700#fdff00#d6ff00#89ff00#2fff00",
   "#9a8262#e8ca93#f0e2a8#fff68f#ffffff",
   "#fdffc2#edefc2#dddfc2#cdcfc2#bdbfc2",
   "#ee729d#fb8c78#f6cb77#ccff83#77f6cb",
   "#32ffc6#65ccd0#9999d9#cc65e3#ff32ec",
   "#f13057#f68118#f9ca00#aef133#19ee9f",
   "#eeeeee#cccccc#bbbbbb#aaaaaa#777777",
   "#eeedee#dddddd#cccccc#bbbbbb#aaaaaa",
   "#d0d8d9#c9d0ce#c6c8c9#bac4c8#b2babf",
};

// https://trendct.org/2016/01/22/how-to-choose-a-label-color-to-contrast-with-background/

unsigned long brighten( unsigned long clr ) {

  int R = (clr & 0xFF0000) >> 16,
      G = (clr & 0x00FF00) >>  8,
      B = (clr & 0x0000FF);

  R = MIN( 0xFF, R+0x10);
  G = MIN( 0xFF, G+0x10);
  B = MIN( 0xFF, B+0x10);

  return( (R<<16)+(G<<8)+B );
}
void set_color256( unsigned long clr, bool BG) {

  int R = (clr & 0xFF0000) >> 16,
      G = (clr & 0x00FF00) >>  8,
      B = (clr & 0x0000FF);

  if ( BG ) {
    printf(  "[%d;2;%03d;%03d;%03dm", FGC, 0, 0, 0);  // Black text
    printf(  "[%d;2;%03d;%03d;%03dm", BGC, R, G, B);
  } else
    printf(  "[%d;2;%03d;%03d;%03dm", FGC, R, G, B);
}
float brightness ( unsigned long clr) {
  float brght;

  int R = (clr & 0xFF0000) >> 16,
      G = (clr & 0x00FF00) >>  8,
      B = (clr & 0x0000FF);

  brght = ( 299.0*R + 587.0*G + 114.0*B ) / 1000;  // This formula is real

//if ( brit > 150 ) printf(  "[%d;2;%03d;%03d;%03dm", BGC, 0x22, 0x35, 0x46);
//else              printf(  "[%d;2;%03d;%03d;%03dm", BGC, 0x7F, 0x7F, 0x7F);
//printf(  "[%d;2;%03d;%03d;%03dm", FGC, R, G, B);
//printf(" %3d ", (int) brit );

  return ( brght );
}
void mycontrast  ( int pal[], int len ) {
  float txt, bkg,
        con;           // contrast
  int   i;

  unsigned
  int rgb = 0X0C1E04, // test values - background color
      tmp;

  bkg = brightness( rgb );

  for (i=0; i < len; ++i ) {
    tmp = pal[i];
    do {

      set_color256(     tmp, B_bkgnd );
      STDOUT("#%06X  ", tmp );
      set_color256( 0xFFFFFF, B_bkgnd );

      txt = brightness( tmp );
      con = MIN( txt, bkg ) / MAX( txt, bkg ) * 100.0;        // I made this up
      STDOUT("%06x: %6.2lf %6.2lf : %6.2lf\n", tmp, txt, bkg, con );
      tmp = brighten( tmp );
    } while ( con > MAX_CON );
  }
  exit( 0 );
}
void bright_pal  ( int pal[], int len ) {
  float txt, bkg,
        con;           // contrast
  int   i;

  unsigned
  int rgb = 0X0C1E04, // test values - background color
      tmp;

  bkg = brightness( rgb );

  for (i=0; i < len; ++i ) {
    tmp = pal[i];
    do {
      txt = brightness( tmp );
      con = MIN( txt, bkg ) / MAX( txt, bkg ) * 100.0;        // I made this up
      if ( con > MAX_CON ) tmp = brighten( tmp );
    } while ( con > MAX_CON );
    pal[i] = tmp;
  }
}
void show_colors ( ) {
  const
  char *pt;
  int i,
      cnt = sizeof( altcolors) / 8;
  unsigned long hex;

  for (i=0; i<cnt; ++i ) {
    pt = altcolors[i];
    set_color256( 0xFFFFFF, B_bkgnd );
    STDOUT("%2d: ", i );
    while ( ( pt=strchr( pt, '#') ) ) {
      pt++;
      hex = strtol( pt, NULL, 16 );
      set_color256(    hex, B_bkgnd );
      STDOUT("#%06lX", hex ); // fflush(stdout);
    }
    STDOUT("\n");
  }
  exit(0);
}
unsigned
int   get_colors ( int ndx, unsigned int *hex ) {
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
void  set_cursor ( bool on) {
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
void  set_color8 ( short stl, short clr) {
  printf("[%d;%dm", stl, clr+31);
}

void  inc_byrow  ( char ch, short *val, unsigned short cycle, int max ) {
  static short cnt = 0;

  if ( *val == -1    )    *val = cnt = 0;
  if (  cnt == cycle ) { (*val)++; *val %= max; }
  cnt %= cycle;
  if ( ch == '\n' ) { 
    ++cnt;
    printf( "[0K");
  }
}
void  inc_bycol  ( short *val, unsigned short cycle, int max ) {
  static short cnt = 0;

  if ( *val == -1 ) *val = cnt = 0;
  if ( cnt == cycle ) (*val)++;
   cnt %= cycle;
  ++cnt;
  *val %= max;
}
void  inc_bywrd  ( char ch, short *val, unsigned short cycle, int max ) {
  static
  char och = '\0';
  static short cnt = 0;

  if ( cnt == cycle ) { (*val)++; *val %= max; }
  cnt %= cycle;
  if ( !isspace( och ) &&  isspace( ch ) ) cnt++;
  och = ch;

}
void  one_line   ( const char *progname ) {
  STDOUT("%-20s: Colorize input\n", progname );
  exit(0);
}
void  help       ( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("colorize text either by character column\n");
  STDERR("\n");
  STDERR("  -c CNT: change color every %d units\n", ccnt );
  STDERR("  -8: 8 bit    colors      [%5s]\n", TF[ !B_256 ]);
  STDERR("  -b: greenbar colors\n");
  STDERR("  -g: rainbow  colors\n");
  STDERR("  -m: metal    colors\n");
  STDERR("  -p [0-%lu]: alternate palettes\n", sizeof( altcolors ) / 8 );
  STDERR("  -B: set background color [%5s]\n", TF[  B_bkgnd ]);
  STDERR("  -f: fix contrast levels  [%5s]\n", TF[  B_fix   ]);
  STDERR("  -r: change color by row  [%5s]\n", TF[  B_row   ]);
  STDERR("  -s palette_list: specify palette index for each column\n");
  STDERR("  -w: change color by word [%5s]\n", TF[  B_wrd   ]);
  STDERR("  -t: show color palettes  [%5s]\n", TF[  B_test  ]);
  STDERR("  -T: show brightness val  [%5s]\n", TF[  B_brght ]);
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
  char *opts=":c:8Bbfgmp:rs:wtTd:uh1";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "cnt",     required_argument, NULL, 'c' },
    { "8bit",          no_argument, NULL, '8' },
    { "bar",           no_argument, NULL, 'b' },
    { "gay",           no_argument, NULL, 'g' },
    { "metal",         no_argument, NULL, 'm' },
    { "palette", required_argument, NULL, 'p' },  // choose a palette
    { "backgrnd",      no_argument, NULL, 'B' },  // set background color
    { "fix",           no_argument, NULL, 'f' },  // adjust brightness of palette selection
    { "row",           no_argument, NULL, 'r' },  // color rows instead of columns
    { "seq",     required_argument, NULL, 's' },
    { "word",          no_argument, NULL, 'w' },
    { "test",          no_argument, NULL, 't' },
    { "bright",        no_argument, NULL, 'T' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",          no_argument, NULL, 'h' },
    { "usage",         no_argument, NULL, 'u' },
    { "oneline",       no_argument, NULL, '1' },
    { NULL,                      0, NULL,  0  }
  };

  unsigned
  int palette[32] = { 0 },
      sz_pal = 0;

  short tseq = -1;

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
          case 'd': debug += dinc; BUGOUT("debug level: %d\n", debug ); dinc <<= 1; break;
          default : BUGOUT("No arg for %c\n", optopt ); break;
        }
        break;

      case 'c': ccnt = strtol( optarg, NULL, 10 );
                break;

      case '8': B_256 = !B_256;
                sz_pal = MAXCOLOR;
                break;
      case 'g': pal_ndx = 0; break;      // rainbow - gay
      case 'b': pal_ndx = 1; break;      // greenbar
      case 'm': pal_ndx = 2; break;      // metal

      case 'p': pal_ndx = strtol( optarg, NULL, 10 );
                break;

      case 's': tseq     = strtol( optarg, NULL, 0 ); sz_seq = append_SEQ( tseq );
                while ( optind < argc &&  ( tseq = strtol( (argv[optind]), NULL, 0 ) ) > 0 ) {
                  sz_seq = append_SEQ( tseq );
                  optind++;
                }
                break;

      case 'B' :B_bkgnd = !B_bkgnd; break;
      case 'f': B_fix   = !B_fix;   break;
      case 'r': B_row   = !B_row;   break;
      case 'w': B_wrd   = !B_wrd;   ccnt = 1; break;
      case 't': B_test  = !B_test;  break;
      case 'T': B_brght = !B_brght; break;

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

  B_tty = isatty(1);    // setcursor fails when stdout is piped
//BUGOUT("tty: %s\n", ttyname(1) );

//if ( SEQ[0] == -1 ) SEQ[0] = tseq;
  if ( debug & 0x0002 ) list_SEQ();

  sz_pal = get_colors( pal_ndx, palette );
  sz_seq = set_SEQ   ( sz_seq, sz_pal, palette );

  if ( debug & 0x0002 ) {
    list_SEQ();
    STDOUT("%2d: sz_seq\n", sz_seq );
  }

  if ( B_test  ) show_colors();
  if ( B_brght ) mycontrast ( SEQ, sz_seq );
  if ( B_fix   ) bright_pal ( SEQ, sz_seq );

  off_t  f_sz = 0;
  char *buf   = NULL,
       *pch;
  short clr = -1,    // color
        stl =  1;    // style

  if ( B_tty ) set_cursor( false );
  if ( optind == argc ) buf = (char *) loadstdin( &f_sz );

//if( B_wrd ) inc_bywrd( ' ', &clr, ccnt, sz_pal ); // solves space/nospace issue on first call

  for (; f_sz || optind < argc; optind++) {         // process remainder of cmdline using argv[optind]

    if ( ! f_sz )
      buf   = (char *) loadfile ( argv[optind], &f_sz );

    pch = buf;
    if( B_wrd && *pch != '\n' ) inc_bywrd( ' ', &clr, ccnt, sz_pal ); // solves space/nospace issue on first call
    while ( f_sz-- ) {
      if      ( B_row ) inc_byrow( *pch, &clr, ccnt, sz_seq );
      else if ( B_wrd ) inc_bywrd( *pch, &clr, ccnt, sz_seq );
      else              inc_bycol(       &clr, ccnt, sz_seq );

      if ( B_256 ) set_color256( SEQ[clr], B_bkgnd );
      else {
        set_color8  ( stl, clr );
        ++clr; clr %= 7;              // increment color
//      ++stl; stl %= 7;              // increment style
      }
      printf("%c", *pch );
      if ( !B_row && *pch == '\n' ) clr = -1;
      ++pch;
    }

    f_sz = 0;
    if ( buf  ) { free( buf  ); buf = NULL; }
  }                                         // for optind

  if ( B_tty ) set_cursor( true  );
  exit(0);
}
