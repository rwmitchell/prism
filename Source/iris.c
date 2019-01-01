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
int  debug =  0;

bool B_o = false;

void set_cursor( bool on) {
  if ( on ) {
    printf("]1337;HighlightCursorLine=yes"); // enable cursor guide in iTerm
    printf("]1337;CursorShape=0");           // set block cursor
  } else {
    printf("]1337;HighlightCursorLine=no"); // Disable cursor guide in iTerm
    printf("]1337;CursorShape=1");          // set vertical cursor
  }
}

void set_color( unsigned short stl, unsigned short clr) {
  printf("[%d;%dm", stl, clr+31);
}


void one_line( const char *progname ) {
  STDOUT("%-20s: Colorize input\n", progname );
  exit(0);
}

void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("colorize text either by column or character\n");
  STDERR("\n");
  STDERR("-o=STRING     (%s : %s)\n", TF[B_o], myopt );
  STDERR("-d INTEGER    (%d)\n", debug );
  STDERR("try again later\n");
  STDERR("\n");

  if ( debug ) helpd( lopts );

  exit(-0);
}

int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt, i,
      longindex=0;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":Xo:d:uh1";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "example", no_argument,       NULL, 'X' },
    { "myopt",   optional_argument, NULL, 'o' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { "oneline", no_argument,       NULL, '1' },
    { NULL,      0,                 NULL,  0  }
  };

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
  int   clr=0,    // color
        stl=0;    // style


  set_cursor( false );
  if ( optind == argc ) buf = (char *) loadstdin( &f_sz );

  for (; f_sz || optind < argc; optind++) {         // process remainder of cmdline using argv[optind]

    if ( ! f_sz )
    buf   = (char *) loadfile ( argv[optind], &f_sz );

    pch = buf;
    while ( f_sz-- ) {
      set_color( stl, clr );
      printf("%c", *pch );
//    ++stl; stl %= 7;
      ++clr; clr %= 7;
      if ( *pch == '\n' ) clr = 0;
      ++pch;
    }

    f_sz = 0;
    if ( buf  ) { free( buf  ); buf = NULL; }
  }                                         // for optind

  set_cursor( true  );
  exit(0);
}
