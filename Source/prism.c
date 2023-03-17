/* prism.c                                        2018-12-31  rwm
 * colorize or strip colors from stdin
 *
  $GLog:
  :GLog$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>    // strcpy()
#include <getopt.h>
#include <ctype.h>     // isdigit()
#include <stdbool.h>   // bool
#include <sys/param.h> // MAX()
#include <sys/time.h>  // gettimeofday()
#include <wchar.h>     // wcwidth()
#include <mylib.h>

const
char *gitid = "$Id$",
     *myid  = "$MyId$",
     *date  = "$Date$",
     *source= "$Source$",
     *auth  = "$Auth$";

const char *TF[]= {"False", "True"};

char myarg[1024],        // temporary optarg value
     FS = ' ';

FP64 offx   = 0.0,       // randomize output colors
     freq_h = 0.23,
     freq_v = 0.1;
SI32 debug  = 0,
     ccnt   = 2;         // contiguous colors
UI64 foregrnd = 0xFF0000;

enum { MAX_SEQ = 64 };
SI32 SEQ[]  = { [0 ... MAX_SEQ] = -1 },
     sz_seq =   0,
     ncol   = 999;       // max columns to color

bool
     B_256   = true,
     B_ONE   = false,    // output a single color
     B_clrz  = true,     // output colorize sequence
     B_bold  = false,
     B_align = false,
     B_bkgnd = false,
     B_fix   = false,
     B_test  = false,
     B_tty   = false,
     B_brght = false,
     B_layer = false,    // keep original colors
     B_strip = false;    // strip colors

#define UNUSED(x) (void)(x)    // silence warning
#define MAX_CON 28.0
#define ARRAY_SIZE(foo) (sizeof(foo) / sizeof(foo[0]))
const unsigned char codes[] = {
   39,  38,  44,  43,  49,  48,
   84,  83, 119, 118, 154, 148,
  184, 178, 214, 208, 209, 203,
  204, 198, 199, 163, 164, 128,
  129,  93,  99,  63,  69,  33 };

enum {
  MAXCOLOR =    6,
  FGC      =   38,         // ansi code to set foreground color
  BGC      =   48,         // ansi code to set background color
};

typedef enum {
  MCOL = 0,
  MROW = 1,
  MWRD = 2,
  MPAR = 3,
  MLOL = 4,
  MFLD = 5,
} Mode_t;
Mode_t mode = MCOL;
SI32  append_SEQ  ( SI32 val ) {                      // append -y args to an array
  SI32 cnt=0,
      *ps = SEQ;

  while ( cnt++ < MAX_SEQ && *ps != -1  ) ++ps;
  *ps = val;

  return( cnt );
}
SI32   set_SEQ    ( SI32 nseq, SI32 npal, UI32 pal[] ) {   // set the color sequence order
  SI32 i,
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

  for ( i=nseq; i<npal; ++i, ++t ) SEQ[i] = t%npal;
  nseq = MAX( nseq, npal );
  for ( i=0; i<nseq; ++i ) SEQ[i] = pal[ SEQ[i] ];

  return( nseq );
}
void   list_SEQ   ( ) {
  SI32 cnt=0,
      *ps = SEQ;

  STDOUT("Y: " );
  while ( cnt++ < MAX_SEQ && *ps != -1  ) STDOUT(" %06X", *ps++ );
  STDOUT("\n");
}
// https://www.color-hex.com
const char *altcolors[] = {
   "#ff005a#fd7674#ffff00#90ee90#8090fb#8564ad#ac84b6",
   "#ff005a#fd7674#ffff00#90ee90#7080fb#75549b#ac84b6",
   "#ff005a#fd7674#ffff00#90ee90#7080fb#76ffff#ff8bff",
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

UI64 brighten    ( UI64 clr ) {

  SI32 R = (clr & 0xFF0000) >> 16,
       G = (clr & 0x00FF00) >>  8,
       B = (clr & 0x0000FF);

  R = MIN( 0xFF, R+0x10);
  G = MIN( 0xFF, G+0x10);
  B = MIN( 0xFF, B+0x10);

  return( (R<<16)+(G<<8)+B );
}
void reset_attr  ( ) {
  printf("\e[0m");
  B_clrz = true;
}
void set_color256( UI64 clr, bool BG) {

  SI32 R = (clr & 0xFF0000) >> 16,
       G = (clr & 0x00FF00) >>  8,
       B = (clr & 0x0000FF);

  if ( B_clrz ) {
    B_clrz = false;

#define COLORS_256
#ifdef  COLORS_256
    if ( B_bold ) printf( "[1;m");
    if ( BG ) {
      printf(  "[%d;2;%03d;%03d;%03dm", FGC, 0, 0, 0);  // Black text
      printf(  "[%d;2;%03d;%03d;%03dm", BGC, R, G, B);
    } else
      printf(  "[%d;2;%03d;%03d;%03dm", FGC, R, G, B);

#else

    R /= 16;
    G /= 16;
    B /= 16;

    if ( BG ) {
      printf(  "[%d;2;%02d;%02d;%02dm", FGC, 0, 0, 0);  // Black text
      printf(  "[%d;2;%02d;%02d;%02dm", BGC, R, G, B);
    } else
      printf(  "[%d;%d;%d;m", R, G, B);
#endif
  }
}

FP32 brightness  ( UI64 clr) {
  FP32 brght;

  SI32 R = (clr & 0xFF0000) >> 16,
       G = (clr & 0x00FF00) >>  8,
       B = (clr & 0x0000FF);

  brght = ( 299.0*R + 587.0*G + 114.0*B ) / 1000;  // This formula is real

//if ( brit > 150 ) printf(  "[%d;2;%03d;%03d;%03dm", BGC, 0x22, 0x35, 0x46);
//else              printf(  "[%d;2;%03d;%03d;%03dm", BGC, 0x7F, 0x7F, 0x7F);
//printf(  "[%d;2;%03d;%03d;%03dm", FGC, R, G, B);
//printf(" %3d ", (SI32) brit );

  return ( brght );
}
void mycontrast  ( SI32 pal[], SI32 len ) {
  FP32 txt, bkg,
       con;            // contrast
  SI32  i;

  UI32 rgb = 0X0C1E04, // test values - background color
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
void bright_pal  ( SI32 pal[], SI32 len ) {
  FP32 txt, bkg,
       con;            // contrast
  SI32 i;

  UI32 rgb = 0X0C1E04, // test values - background color
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
  SI32 i,
       cnt = ARRAY_SIZE( altcolors );
  UI64 hex;

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
UI32 str2hex     ( const char *str, UI32 *hex ) {
  const
  char *pt;
  SI32 pos = 0;

  pt = str;
  while ( ( pt=strchr( pt, '#') ) ) {
    pt++;
    hex[pos++] = strtol( pt, NULL, 16 );
  }

  return( pos );
}
UI32 get_colors  ( SI32 ndx, UI32 *hex ) {
  SI32 cnt = ARRAY_SIZE( altcolors ) - 1;

  if ( ndx < 0 || ndx > cnt ) {
    BUGERR("ndx %d is invalid, expected 0 to %d\n", ndx, cnt );
    exit( __LINE__ );
  }

  return( str2hex( altcolors[ndx], hex ) );
}
void set_cursor  ( bool on) {
  if ( on ) {
    printf("]1337;HighlightCursorLine=yes"); // enable cursor guide in iTerm
//  printf("]1337;CursorShape=0");           // set block     cursor
    printf("]1337;CursorShape=2");           // set underline cursor
    printf("[%d;m", 0 );  // not bold
  } else {
    printf("]1337;HighlightCursorLine=no"); // Disable cursor guide in iTerm
    printf("]1337;CursorShape=1");          // set vertical cursor
    printf("[%d;m", 1 );  // bold
  }
}
void set_color8  ( SI16 stl, SI16 clr) {
  printf("[%d;%dm", stl, clr+31);
}
void inc_bypar   ( char ch, SI32 *val, UI16 cycle, SI32 max ) {
  static SI16 cnt = 0,
              par = 0;        // total paragraphs
  static char och = '\0';

  if ( *val == -1    )    *val = cnt = 0;
  if (  cnt == cycle ) { (*val)++; *val %= max; }
  if ( cycle == 0 ) cycle = 1;               // make sure cycle is not zero
  cnt %= cycle;
  if ( ch == '\n' && och == '\n' ) {
    cnt++;
    par++;
    printf( "[0K");
  }
  och = ch;
  if ( par >= ncol ) *val = -1;
}
void inc_byrow   ( char ch, SI32 *val, UI16 cycle, SI32 max ) {
  static SI32 cnt = 0,
              row = 0;

  if ( *val == -1    )    *val = cnt = 0;
  if (  cnt == cycle ) { (*val)++; *val %= max; }
  if ( cycle == 0 ) cycle = 1;               // make sure cycle is not zero
  cnt %= cycle;
  if ( ch == '\n' ) {
    cnt++;
    row++;
    printf( "[0K");
  }
  if ( row > ncol ) *val = -1;
}
void inc_byfld   ( char ch, SI32 *val, UI16 cycle, SI32 max ) {
  static SI32 cnt = 0,
              pos = 0,
              fld = 0,
              fpl = 0,
              fpm[64] = { 0 };    // max pos for each fld

  pos++;
  if ( ch == '\n' ) {  fld = fpl = 0; cnt = 1; }

  if ( *val == -1    )    *val = pos = cnt = 0;
  if (  cnt == cycle ) { (*val)++; *val %= max; }
  if ( cycle == 0 ) cycle = 1;               // make sure cycle is not zero
  cnt %= cycle;
  if ( ch == FS && fld <= ncol ) {
    cnt++;
    fld++;
    fpl++;
    if ( fpm[fld] == 0 ) fpm[fld] = pos;
    fpm[fld] = MAX( fpm[fld], MIN( fpm[fld]+4, pos ) );           // limit increasing field separator position
    if ( B_align && fpm[fld] >= pos) STDOUT("%*s", fpm[fld] - pos, "");
    pos = 0;
  }
  if ( fpl >= ncol ) *val = -1;
}
void inc_bycol   ( char ch, SI32 *val, UI16 cycle, SI32 max ) {
  static SI32 cnt = 0,
              cpl = 0;    // columns per line

  if ( *val == -1 ) *val = cnt = 0;
  if ( cnt == cycle ) { (*val)++; cpl++; }
  if ( ch == '\n' )  cpl = 0;
  if ( cycle == 0 ) cycle = 1;               // make sure cycle is not zero
  cnt %= cycle;
  cnt++;
  if ( max   == 0 ) max   = 1;               // make sure cycle is not zero
  *val %= max;
  if ( cpl >= ncol ) *val = -1;
}
void inc_bylol   ( char ch, SI32 *val ) {
  static SI32 i = 0,
             l = 0;
  if ( ch == '\n' ) { l++; i=0; }
  else {
#ifdef __APPLE__
    SI32 nval = offx * ARRAY_SIZE(codes) + (SI32)((i += wcwidth(ch)) * freq_h + l * freq_v);
#else
    SI32 nval = offx * ARRAY_SIZE(codes) + (SI32)((i += 1          ) * freq_h + l * freq_v);
#endif

    if ( *val != nval )
      wprintf(L"\033[38;5;%hhum", codes[(*val = nval) % ARRAY_SIZE(codes)]);
  }
}
void inc_bywrd   ( char ch, SI32 *val, UI16 cycle, SI32 max ) {
  static char och = '\0';
  static SI32 cnt = 0,
              wpl = 0;             // words per line

  if ( och == '\n' ) {  wpl = 0; cnt = 1; }

  if ( cnt == cycle ) { (*val)++; *val %= max; ++wpl; }
  if ( cycle == 0 ) cycle = 1;     // make sure cycle is not zero
  cnt %= cycle;
  if ( !isspace( och ) &&  isspace( ch ) ) cnt++;
  och = ch;
  if ( wpl > ncol ) *val = -1;
}
char mygetch     ( bool set, void *buf ) {
  UNUSED( set );                  // args need to match mybufch()
  UNUSED( buf );
//static char *pch;               // these lines are not used directly,
//if ( set ) pch = (char *) buf;  // just staying compatible to mybufch()

  return( getchar() );
}
char mybufch     ( bool set, void *buf ) {
  static char *pch;

  if ( set ) pch = (char *) buf;
  else ++pch;

  return( *pch );
}
// "Borrowed" from lolcat.c - START
static
void find_escape_sequences(wint_t c, SI32* state, bool *on) {
  static char esc[32] = { '\0' };
  static char *pc = esc;

  if (c == '\033') { /* Escape sequence YAY */
    *state = 1;
  } else if (*state == 1) {
    if ( c == ']' ) {
      *state = 3;   // assume a ]1337 escape code
    }
    else {
      *(pc++) = c;
      if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
        *state = 2;
    }
  } else if ( *state == 2 ) {     // do not reset if *state == 3
    *state = 0;
    B_clrz = true;
  }

  // escape off sequence can be either \[[m or \[[0;0m

  if ( *state == 2 ) {
    pc--;
    if ( *pc == 'm' || *pc == 'n' ) {
      SI32 f = 0, b = 0, s = 0;
      sscanf( esc, "[%d%*c%d%*c%d", &b, &f, &s );
      *on =  b+f ? true : false;
    }
    memset( esc, '\0', 31 );      // clear
    pc = esc;                     // and reset
  } else if ( *state == 3 ) {
    if ( c == '\a' ) {            // \a is ^G, end of 1337 code
      *state = 2;
    }
  }
}
// "Borrowed" from lolcat.c - END
void one_line    ( const char *progname ) {
  STDOUT("%-20s: Colorize input\n", progname );
  exit(0);
}
void help        ( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n", RWM_VERSION );
  STDERR("%s\n%s\n\n", gitid, source );
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("colorize text either by character, column, row, or field\n");
  STDERR("\n");
  STDERR("  -c CNT: change color every %d units\n", ccnt );
  STDERR("  -8: 8 bit    colors      [%5s]\n", TF[ !B_256 ]);
  STDERR("  -b: greenbar colors\n");
  STDERR("  -g: rainbow  colors\n");
  STDERR("  -l: lolcat   colors\n");
  STDERR("  -L: leave existing escape codes along\n" );
  STDERR("  -H: lolcat horizontal frequency :%.3lf\n", freq_h );
  STDERR("  -V: lolcat vertical   frequency :%.3lf\n", freq_v );
  STDERR("  -m: metal    colors\n");
  STDERR("  -n CNT: max of %d items to colorize\n", ncol );
  STDERR("  -p [0-%lu]: alternate palettes\n", ARRAY_SIZE( altcolors ) );
  STDERR("  -B: set background color [%5s]\n", TF[  B_bkgnd ]);
  STDERR("  -f: fix contrast levels  [%5s]\n", TF[  B_fix   ]);
  STDERR("  -P: change color by paragraph\n" );
  STDERR("  -r: change color by row\n" );
  STDERR("  -s palette_list: specify palette index for each column\n");
  STDERR("  -S: strip color\n");
  STDERR("  -w: change color by word\n");
  STDERR("  -a: align on SEP [%5s]\n", TF[  B_align ]);
  STDERR("  -F SEP : change color by field [%c]\n", FS);
  STDERR("  -t: show color palettes  [%5s]\n", TF[  B_test  ]);
  STDERR("  -T: show brightness val  [%5s]\n", TF[  B_brght ]);
  STDERR("  -d INTEGER    (%d)\n", debug );
  STDERR("\n");
  STDERR("  -bold : set output text to bold\n");
  STDERR("  -foreground %06lX : set single color for output\n", foregrnd );
  STDERR("\n");
  STDERR("A custom palette can be set using the environment variable 'PRISM'. \n");
  STDERR("Ex: export PRISM=\"#00FF00#00E000#00C000#00A000#008000\"\n");
  STDERR("or  setenv PRISM \"#00FF00#00E000#00C000#00A000#008000\"\n");

  if ( debug ) RMhelpd( lopts );

  exit(-0);
}
SI32 main(SI32 argc, char *argv[]) {
  SI32 errflg = 0,
       dinc   = 1,                // debug incrementor
       opt,
       pal_ndx  = -1,
       longindex=  0;
  UI16 optcnt = 1;
  bool B_have_arg = true;
  extern SI32  optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":c:8aBbfF:glLmn:p:Prs:SwtTH:V:Zd:uh1";   // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "cnt",       required_argument, NULL, 'c' },
    { "8bit",            no_argument, NULL, '8' },
    { "align",           no_argument, NULL, 'a' },
    { "bar",             no_argument, NULL, 'b' },
    { "gay",             no_argument, NULL, 'g' },
    { "metal",           no_argument, NULL, 'm' },
    { "num",       required_argument, NULL, 'n' },
    { "palette",   required_argument, NULL, 'p' },  // choose a palette
    { "backgrnd",        no_argument, NULL, 'B' },  // set background color
    { "foreground",optional_argument, NULL, 202 },
    { "bold",            no_argument, NULL, 203 },  // set background color
    { "fix",             no_argument, NULL, 'f' },  // adjust brightness of palette selection
    { "row",             no_argument, NULL, 'r' },  // color rows instead of columns
    { "seq",       required_argument, NULL, 's' },
    { "strip",           no_argument, NULL, 'S' },  // strip colors from input, do not add
    { "paragraph",       no_argument, NULL, 'P' },
    { "word",            no_argument, NULL, 'w' },
    { "field",     required_argument, NULL, 'F' },
    { "lol",             no_argument, NULL, 'l' },  // duplicate lolcat
    { "layer",           no_argument, NULL, 'L' },  // keep original non-white colors
    { "test",            no_argument, NULL, 't' },
    { "bright",          no_argument, NULL, 'T' },
    { "horiz",     required_argument, NULL, 'H' },
    { "vert",      required_argument, NULL, 'V' },
    { "notty",           no_argument, NULL, 'Z' },   // disable tty
    { "version",         no_argument, NULL, 201 },
    { "debug",     optional_argument, NULL, 'd' },
    { "help",            no_argument, NULL, 'h' },
    { "usage",           no_argument, NULL, 'u' },
    { "oneline",         no_argument, NULL, '1' },
    { NULL,                        0, NULL,  0  }
  };

  char *env_col = getenv("PRISM");

  UI32 palette[32] = { 0 },
      sz_pal = 0;

  SI32 tseq = -1;

  char ( *myread)( bool, void * );

  struct timeval tv;
  gettimeofday(&tv, NULL);
  offx = (tv.tv_sec % 300) / 300.0;

  B_tty = isatty( STDOUT_FILENO );    // setcursor fails when stdout is piped

  // parse command line options
  SI32 has_arg = 0;
  while ( ( opt=getopt_long_only(argc, argv, opts, longopts, &longindex )) != EOF ) {

    B_have_arg = true;
    memset( myarg, '\0', 1024 );      // reset

    if ( longindex ) {                        // we got a longopt
      opt     = longopts[longindex].val;      // set opt to short opt value
      has_arg = longopts[longindex].has_arg;

      // optarg is already set if -option=value was used
      // optarg is NULL        if -option value was used
      if ( has_arg == 2 && ! optarg ) optarg = argv[optind];
    }

    // Pre-Check
    if ( optarg ) {                // only check if not null
      switch (opt) {               // check only args with possible STRING options
        case 202:
        case 'd':
          if ( !optarg || *optarg == '\0' ) {
            if ( argv[optind] == NULL ) {
              BUGOUT("next arg is also NULL\n");
            } else if ( *optarg == '-' ) {  // optarg is actually the next option
              if ( !longindex ) optind--;   // only decrement if not longopt
              B_have_arg = false;
            } else {
              strcpy(myarg, argv[optind++]);
              if (  longindex ) optind++;   // increment twice for longopt?
            }
          }
          break;
      }
    } else
      B_have_arg = false;          // optarg was null

    if ( opt == ':' ) {            // check for missing required args
      switch( optopt ) {
        case 'c':
        case 'n':
        case 'p':
        case 's':
        case 'F':
        case 'H':
        case 'V':
          // if there is no arg for a required longopt, longindex is not set
          // thus cannot show original 'name'
          GTFOUT( "Missing required argument for '%c'\n", optopt )
          break;
      }
    }

    if ( opt == ':' ) opt = optopt;

    // Normal Check
    switch (opt) {
      case ':':              // check optopt for previous option
        switch( optopt ) {
          case 'd': debug += dinc;
                    dinc <<= 1;
                    break;
          default : BUGOUT("No arg for %c\n", optopt ); break;
        }
        break;

      case 'c': ccnt = strtol( optarg, NULL, 10 );
                break;

      case '8': B_256 = !B_256;
                sz_pal = MAXCOLOR;
                break;
      case 'g': pal_ndx = 0; break;      // rainbow - gay
      case 'b': pal_ndx = 4; break;      // greenbar
      case 'm': pal_ndx = 5; break;      // metal

      case 'n': ncol = strtol( optarg, NULL, 10 );
                break;

      case 'p': pal_ndx = strtol( optarg, NULL, 10 );
                break;

      case 's': tseq     = strtol( optarg, NULL, 0 ); sz_seq = append_SEQ( tseq );
                while ( optind < argc &&  ( tseq = strtol( (argv[optind]), NULL, 0 ) ) > 0 ) {
                  sz_seq = append_SEQ( tseq );
                  optind++;
                }
                break;

      case 'a' :B_align = !B_align;    break;
      case 'B' :B_bkgnd = !B_bkgnd;    break;
      case 'f': B_fix   = !B_fix;      break;
      case 'P': mode = MPAR; ccnt = 1; break;
      case 'r': mode = MROW;           break;
      case 'w': mode = MWRD; ccnt = 1; break;
      case 't': B_test  = !B_test;     break;
      case 'T': B_brght = !B_brght;    break;

      case 'L': B_layer = !B_layer;    break;
      case 'S': B_strip = !B_strip;    break;

      case 'F': if ( optarg ) {
                  mode = MFLD; ccnt = 1; FS = optarg[0];
                }
                break;
      case 'l': mode = MLOL; ccnt = 1; B_256 = false;  break;
      case 'H': freq_h = strtod( optarg, NULL );       break;
      case 'V': freq_v = strtod( optarg, NULL );       break;

      case 'Z': B_tty  = false    ;    break;

      case 201:
                STDOUT( "%s\n", RWM_VERSION );
                STDOUT( "%s\n", gitid       );
                STDOUT( "%s\n", myid        );
                STDOUT( "%s\n", source      );
                STDOUT( "%s\n", date        );
                STDOUT( "%s\n", auth        );
                exit(0);
                break;

      case 202:
        if ( B_have_arg ) {
          foregrnd = strtol(optarg, NULL, 16 );
          if ( foregrnd == 0 ) {        // we didn't get a number
            foregrnd = 0xFF0000;
            B_have_arg = false;
            --optind;
          }
        }
        B_ONE = true;
        break;

      case 203: B_bold = !B_bold; break;

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

      case '1': one_line( argv[0] ); break;

      case 'h':
      default :
        ++errflg;
        break;
    }
    optcnt += B_have_arg ? 2 : 1;
    longindex = 0;   // Reset value
    has_arg = 0;
  }

  if (errflg) help(argv[0], opts, longopts);

//if ( SEQ[0] == -1 ) SEQ[0] = tseq;
  if ( debug & 0x0002 ) list_SEQ();

  if ( pal_ndx < 0 ) {
    if ( env_col ) sz_pal = str2hex( env_col, palette );
    else pal_ndx = 0;    // force pal_ndx to rainbow only
  }

  if ( ! sz_pal )
  sz_pal = get_colors( pal_ndx, palette );        // no indent for alignment

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
       *pch,
         ch;
  SI32 clr = -1,    // color
       stl =  1,    // style
       oclr;

  if ( B_tty ) set_cursor( false );
  if ( optcnt >= argc ) { myread = mygetch; f_sz = 1; }  // read from stdin

//if( B_wrd ) inc_bywrd( ' ', &clr, ccnt, sz_pal ); // solves space/nospace issue on first call

  for (; f_sz || optind < argc; optind++) {         // process remainder of cmdline using argv[optind]
    SI32 escape_state = 0;
    bool on = false;

    if ( ! f_sz ) {
      buf   = (char *) RMloadfile ( argv[optind], &f_sz, false );
      mybufch( true, buf );
      myread = mybufch;
    }

    reset_attr();                                   // clear any cruft

    pch = &ch;
    *pch = myread( false, (void *) NULL );
    if( mode == MWRD && *pch != '\n' )
      inc_bywrd( ' ', &clr, ccnt, sz_pal );         // solves space/nospace issue on first call
    while ( *pch > 0
#ifndef __APPLE__
        && *pch < 255
#endif
        ) {

      find_escape_sequences( *pch, &escape_state, &on );

      if (!escape_state) {

        if ( B_layer && on ) printf( "%c", *pch );
        else if ( !B_layer || !on ) {

          if ( B_strip ) printf( "%c", *pch );
          else {

            if ( B_ONE )
                clr = B_clrz;
            else {
              oclr = clr;
              switch ( mode ) {
                case MCOL: inc_bycol( *pch, &clr, ccnt, sz_seq ); break;
                case MROW: inc_byrow( *pch, &clr, ccnt, sz_seq ); break;
                case MWRD: inc_bywrd( *pch, &clr, ccnt, sz_seq ); break;
                case MPAR: inc_bypar( *pch, &clr, ccnt, sz_seq ); break;
                case MFLD: inc_byfld( *pch, &clr, ccnt, sz_seq ); break;
                case MLOL: inc_bylol( *pch, &clr ); break;
              }
              if ( oclr != clr ) B_clrz = true;
            }

            if ( clr == -1 ) { reset_attr(); on = false; escape_state = 0; }
            else {
              if ( B_ONE ) set_color256( foregrnd, B_bkgnd );
              else
              if ( B_256 ) set_color256( SEQ[clr], B_bkgnd );
              else {
                if ( mode != MLOL ) set_color8  ( stl, clr );
        //      ++clr; clr %= 7;              // increment color
        //      ++stl; stl %= 7;              // increment style
              }
            }
            printf("%c", *pch );
            if ( ( mode != MROW && mode != MPAR && mode != MLOL ) && *pch == '\n' ) {
              clr = -1;
              on  = false;
              escape_state = 0;
            }
          }
        }
//    } else if ( escape_state == 3 ) {
//      printf( "^%c$", *pch );
      } else if ( B_layer || escape_state == 3 )
        printf( "%c", *pch  );                // print escape codes if layering

      *pch = myread( false, (void *) NULL );
    }

    f_sz = 0;
    if ( buf  ) { free( buf  ); buf = NULL; }
  }                                         // for optind

  if ( B_tty ) set_cursor( true  );
  exit(0);
}
