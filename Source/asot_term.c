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

typedef enum { False=0, True=1 } Bool;

const char *TF[]= {"False", "True"};
const char *white = " \t\n";

char  myarg[1024];
int   debug =  0;
float msped =  1.0;

Bool B_o = False;

#define BUGOUT( FMT, ... ) { \
  fprintf(stdout, "%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
}
#define BUGERR( FMT, ... ) { \
  fprintf(stderr, "%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
}
#define BUGNUL( FMT, ... ) { \
}                // Do nothing
#define STDOUT( FMT, ... ) { \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
}

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
      speed;

  speed = (int) (1000 * msped);

  while ( (ch = fgetc( myin )) != EOF ) {
    if ( strchr( str, ch ) ) {
#ifdef DEBUG
      fprintf(stdout, "%c: ", ch );
#endif
      while (  !strchr( white, ch ) && ch != str[i] ) {
#ifdef  DEBUG
        fprintf(stdout, "%c", str[i++] );
#else
        fprintf(stdout, "%c", str[i++] );
#endif
        fflush(stdout);
        usleep(speed);
        i = i % cnt;
      }
#ifdef  DEBUG
      fprintf(stdout, " *%c*\n", str[i] );
#endif
    }
    fprintf(stdout, "%c", ch );
  }
  return( 0 );
}
// ############################################################

void usage( char *progname, char *opt) {
  fprintf(stderr, "%s %s\n", __DATE__, __TIME__ );
  fprintf(stderr, "%s\n\n", cvsid);
  fprintf(stderr, "%s [-%s]\n", progname, opt);
  fprintf(stderr, "-o STRING     (%s : %s)\n", TF[B_o], myarg );
  fprintf(stderr, "-d INTEGER    (%d)\n", debug );
  fprintf(stderr, "try again later\n");
  exit(-0);
}

int main(int argc, char *argv[]) {
  int debug =0,
      errflg=0,
      dinc=1;                // debug incrementor
  int c;
  char *opts=":Xo:s:d:h";    // Leading : makes all :'s optional
  extern int   optind;
  extern char *optarg;

  extern int   optind,
               optopt;
  extern char *optarg;

  strcpy(myarg, "defval");

  // parse command line options
  while (( c=getopt( argc, argv, opts)) != EOF) {
    switch (c) {   // check only args with possible options
      case 'o':
      case 'd':
        if ( *optarg == '-' ) {
          printf("Got hyphen for %c, rewinding\n", c );
          --optind;
        }
        break;
    }

    switch (c) {
      case ':':     // check optopt for previous option
        printf("Got a Colon for: %c\n", optopt );
        switch( optopt ) {
          case 'o': B_o = !B_o;    printf("No arg for o (%s)\n", myarg ); break;
          case 'd': debug += dinc; printf("debug level: %d\n", debug ); dinc <<= 1; break;
          default : printf("No arg for %c\n", optopt ); break;

        }
        break;

      case 's':     // set speed multiplier
        msped = strtof( optarg, NULL );
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

      case 'h':
      default :
        ++errflg;
        break;
    }
  }

  if (errflg) usage(argv[0], opts);

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
