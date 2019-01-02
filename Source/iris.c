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
    },
    // https://www.color-hex.com
    metal2[] = { 0xeeeeee, 0xcccccc, 0xbbbbbb, 0xaaaaaa, 0x777777 },
    metal3[] = { 0xd0d8d9, 0xc9d0ce, 0xc6c8c9, 0xbac4c8, 0xb2babf },
    fire1 [] = { 0xa70000, 0xdb1414, 0xec5300, 0xf27038, 0xff8d00 },
    fire2 [] = { 0x9c2000, 0xb44000, 0xd86f05, 0xf09d00, 0xffc100 },
    green1[] = { 0x00ff00, 0x00e000, 0x00c000, 0x00a000, 0x008000 },
    green2[] = { 0x1d945b, 0x1b9f5f, 0x1ba964, 0x1ab268, 0x17ba6b },
    green3[] = { 0x72e736, 0x15ff00, 0x0fb200, 0x69db93, 0x54c372 },
    green4[] = { 0x6eff98, 0x59d85e, 0x38a44c, 0x30864d, 0x186038 },
    rainb1[] = { 0xff005a, 0xffdc00, 0xd9ff00, 0x00ff1c, 0x005cff },
    rainb2[] = { 0xff0000, 0xff8100, 0xfff400, 0x22ff00, 0x0045ff },
    rainb3[] = { 0xff0000, 0xffff00, 0x00ff00, 0x0000ff, 0xff00ff },
    tech1 [] = { 0x244ca3, 0xe56f1e, 0x247a3b, 0xd22626, 0x568dd8 },
    grey1 [] = { 0xccd8dc, 0xb7c2c6, 0xa3acb0, 0x8c979a, 0x7a8184 },
    bronz1[] = { 0xb87f0d, 0xb07504, 0xbd6700, 0xb06304, 0xa95b0e },
    orang1[] = { 0xd4582c, 0xfb7125, 0xff861d, 0xff9529, 0xfdab3a },
    orang2[] = { 0x7d0000, 0xcd3700, 0xcd7f32, 0xff8c00, 0xffa500 };

/* Colors
    metal2[] = { #eeeeee, #cccccc, #bbbbbb, #aaaaaa, #777777 }
    metal3[] = { #d0d8d9, #c9d0ce, #c6c8c9, #bac4c8, #b2babf }
    fire1 [] = { #a70000, #db1414, #ec5300, #f27038, #ff8d00 }
    fire2 [] = { #9c2000, #b44000, #d86f05, #f09d00, #ffc100 }
    green1[] = { #00ff00, #00e000, #00c000, #00a000, #008000 }
    green2[] = { #1d945b, #1b9f5f, #1ba964, #1ab268, #17ba6b }
    green3[] = { #72e736, #15ff00, #0fb200, #69db93, #54c372 }
    green4[] = { #6eff98, #59d85e, #38a44c, #30864d, #186038 }
    rainb1[] = { #ff005a, #ffdc00, #d9ff00, #00ff1c, #005cff }
    rainb2[] = { #ff0000, #ff8100, #fff400, #22ff00, #0045ff }
    rainb3[] = { #ff0000, #ffff00, #00ff00, #0000ff, #ff00ff }
    tech1 [] = { #244ca3, #e56f1e, #247a3b, #d22626, #568dd8 }
    grey1 [] = { #ccd8dc, #b7c2c6, #a3acb0, #8c979a, #7a8184 }
    bronz1[] = { #b87f0d, #b07504, #bd6700, #b06304, #a95b0e }
    orang1[] = { #d4582c, #fb7125, #ff861d, #ff9529, #fdab3a }
    orang2[] = { #7d0000, #cd3700, #cd7f32, #ff8c00, #ffa500 }

#eeeeee#d0d8d9#ccd8dc
#cccccc#c9d0ce#b7c2c6
#bbbbbb#c6c8c9#a3acb0
#aaaaaa#bac4c8#8c979a
#777777#b2babf#7a8184

#a70000#9c2000#b87f0d#d4582c#7d0000
#db1414#b44000#b07504#fb7125#cd3700
#ec5300#d86f05#bd6700#ff861d#cd7f32
#f27038#f09d00#b06304#ff9529#ff8c00
#ff8d00#ffc100#a95b0e#fdab3a#ffa500

#00ff00#1d945b#72e736#6eff98
#00e000#1b9f5f#15ff00#59d85e
#00c000#1ba964#0fb200#38a44c
#00a000#1ab268#69db93#30864d
#008000#17ba6b#54c372#186038

#ff005a#ff0000#ff0000#244ca3
#ffdc00#ff8100#ffff00#e56f1e
#d9ff00#fff400#00ff00#247a3b
#00ff1c#22ff00#0000ff#d22626
#005cff#0045ff#ff00ff#568dd8

*/

int *palettes[] = { metal2,  metal3,
                    fire1,   fire2,
                    green1,  green2, green3, green4,
                    rainb1,  rainb2, rainb3,
                    tech1,   grey1, bronz1,
                    orang1, orang2
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
  STDERR("  -p [0-%lu]: alternate palettes\n", sizeof( palettes ) / 8 );
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
      x,
      longindex=0;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":o:c:8bgmp:rwd:uh1";      // Leading : makes all :'s optional
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
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { "oneline", no_argument,       NULL, '1' },
    { NULL,      0,                 NULL,  0  }
  };

  int *palette = rainbow,
       sz_pal  = sizeof( rainbow  ) / 4,
       sz_all  = sizeof( palettes ) / 8;

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

      case 'p': x = strtol( optarg, NULL, 10 );
                if ( x < 0 || x > sz_all ) {
                  BUGERR("Invalid palette %d, expected 0 to %d\n", x, sz_all );
                  exit( __LINE__ );
                }
                palette = palettes[x];
                sz_pal  = 5;
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
//BUGOUT("palette: %lu\n", sizeof( palettes )/8);

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
