const
char *cvsid = "$Id$";
/* asot_term: As Seen On Tv TERMinal output
 * reads a char from stdin
 * if the char matches the current char in myascii, print it
 * if not, print the current myascii char and a backspace
 *
 * This is from a stupid special effect I saw briefly in the
 * movie "The Martian" with Matt Damon
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // usleep()
#include <string.h>  // strcpy()
#include <getopt.h>
#include <ctype.h>   // isprint()
#include <sys/ioctl.h>
#include <stdbool.h>
#include "bugout.h"

const char *TF[]= {"False", "True"};
const char *white = " \t\n";

char  myarg[1024];
int   debug =   0,
      speed =  10;
struct winsize wsize;


bool B_o = false;

// ############################################################
int get_printable( char *str ) {
  int cnt = 0,
      i;

  for ( i=0; i<=0xFF; ++i ) {
    if ( isprint( i ) ) {
      *str = i;
      ++cnt;
      ++str;
    }
  }

  return( cnt );
}
int asot_term( FILE *myin, char *str, int cnt ) {
  int i=0,
      ch,
      col=1,
      och;

  while ( (ch = fgetc( myin )) != EOF ) {
    if ( strchr( str, ch ) ) {
      while (  !strchr( white, ch ) && ch != str[i] ) {
        if ( col > 0 ) {
          STDOUT( "%c", str[i++] );
        } else {
          STDOUT( "");
          ++i;
        }
        fflush(stdout);
        usleep(speed*100);
        i = i % cnt;
      }
    }

//  if ( !strchr( white, ch ) ) {
    if ( ch != '\n' ) {
      if (col >= wsize.ws_col-0 ) {
        STDOUT("%c%c\n", och, ch);
        col = 0;
      } else {
        STDOUT( "%c", ch );
      }
    } else {
      col = 0;
      STDOUT( "%c", ch );
    }

    ++col;
    och = ch;
  }
  return( 0 );
}
// ############################################################

void usage( const struct option longopts[] ) {
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
void help( char *progname, const char *opt, const struct option lopts[]) {
  int i;
  STDERR( "%s %s\n", __DATE__, __TIME__ );
  STDERR( "%s\n\n", cvsid);
  STDERR( "%s [-%s]\n", progname, opt);
  STDERR( "-s microseconds*100  (%d)\n", speed );
  STDERR( "-d INTEGER    (%d)\n", debug );
  STDERR( "try again later\n\n");

  STDERR("%2s %-15s %4s %4s %c\n",
      "  ",
      "Name",
      "arg",
      "val",
      '-' );

  for (i=0; lopts[i].name != NULL; ++i ) {
    STDERR("%2d %-15s %4d %4d %c",
      i,
      lopts[i].name,
      lopts[i].has_arg,
      lopts[i].val,
      isprint(lopts[i].val) ? lopts[i].val : ' '
    );
    if ( lopts[i].flag != NULL ) {
      STDERR(" Pointer: %d", *lopts[i].flag );
    }
    STDERR("\n");
  }

  exit(-0);
}

int main(int argc, char *argv[]) {
  int debug =0,
      errflg=0,
      dinc=1;                // debug incrementor
  int opt,
      longindex;
  const
  char *opts=":o:s:d:hu";    // Leading : makes all :'s optional
  extern char *optarg;
  extern int   optind,
               optopt;

  const struct option longopts[] = {
    { "debug",   required_argument, NULL, 'd' },
    { "speed",   required_argument, NULL, 's' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
    { NULL,      0,                 NULL,  0  }
  };

  strcpy(myarg, "defval");

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize );
//BUGOUT("row: %d\n", wsize.ws_row );
//BUGOUT("col: %d\n", wsize.ws_col );

  // parse command line options
  while ( ( opt=getopt_long_only(argc, argv, opts, longopts, &longindex )) != EOF ) {
    switch (opt) {   // check only args with possible options
      case 'o':
      case 'd':
        if ( *optarg == '-' ) {
          printf("Got hyphen for %c, rewinding\n", opt );
          --optind;
        }
        break;
    }

    switch (opt) {
      case ':':     // check optopt for previous option
        printf("Got a Colon for: %c\n", optopt );
        switch( optopt ) {
          case 'o': B_o = !B_o;    printf("No arg for o (%s)\n", myarg ); break;
          case 'd': debug += dinc; printf("debug level: %d\n", debug ); dinc <<= 1; break;
          default : printf("No arg for %c\n", optopt ); break;

        }
        break;

      case 's':     // set speed
        speed = strtol( optarg, NULL, 10 );
        break;

      case 'o':
        B_o = !B_o;
        printf("option: o\n");
        if ( *optarg != '-' ) {
          strcpy(myarg, optarg);
          printf("optarg = %c(%s)\n", *optarg, myarg);
        } else
          printf("using previous value: %s\n", myarg );
        break;

      case 'd':
        if ( *optarg != '-' )
          debug = strtol(optarg, NULL, 10 );
        else {
          debug += dinc; dinc <<= 1;
        }
        printf("debug level: %d\n", debug );
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

  if (errflg) help( argv[0], opts, longopts);

  char myascii[256];  // this will be a list of printable characters
  int  myascii_cnt;   // number of chars in myascii

  myascii_cnt = get_printable( myascii );

  if ( optind < argc ) {
    FILE *in;
    for (; optind < argc; optind++) {         // use argv[optind]

      if ( ( in = fopen(argv[optind], "r")) != NULL ) {
        asot_term( in, myascii, myascii_cnt );
        fclose( in );
      } else {
        BUGERR("Unable to fopen %s\n", argv[optind] );
      }
    }                                         // for optind

  } else {
    BUGNUL( ">>>%s<<<\n", white );
    BUGNUL( "%3d:\n###%s###\n", myascii_cnt, myascii );

    asot_term( stdin, myascii, myascii_cnt );
  }

  exit(0);
}
