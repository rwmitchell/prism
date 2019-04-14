// ttl: take too long - announce job that takes too long to complete
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strcpy()
#include <getopt.h>
#include <ctype.h>   // isdigit()
#include <stdbool.h> // bool
#include "io.h"
#include "bugout.h"
#include "helpd.h"

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0;

bool B_o = false;

int *parse_line( const char *line ) {
  const
  char *pc = line;
  int i=0;

  static
  int rv[10];    // max of 10 return elements
  
  while ( ( pc = strchr( pc, '$' )) != NULL && i<10 ) {
    ++pc;               // move to next char;
    rv[i++] = *pc-'0';
  }

  return( rv );
}
int restring( char *str ) {
  char *pc = str;
  int cnt=0;

  while ( (pc=strchr( pc, '$' )) != NULL ) {
    cnt++;
    *pc = '%';
     pc++;
    *pc = 's';
  }
  return( cnt );
}
void load_file( const char *fname ) {
  FILE *F_in;
  int rc,
     *arr;
  char line[512],
      *cmd,
      *msg,
      *pc;


  if ( ( F_in=fopen( fname, "r" ) ) == NULL ) {
    BUGOUT( "%s: Unable to open", fname );
    exit( __LINE__ );
  }

  while ( (rc=fgetl(F_in, line, 511) ) > 0 ) {
    pc = strchr( line, '#' );
    if ( pc ) *pc = '\0';
    if ( pc-line > 0 ) {
      cmd = line;
      msg = strchr( line, ':' );
      *msg = '\0';
      msg++;
      arr = parse_line( msg );

      rc = restring( msg );
      STDOUT("%2d: (%s):%s", rc, cmd, msg );
      for (int i=0; i<rc; ++i ) STDOUT(" %d", arr[i] );
      STDOUT("\n");
    }
  }
}
void one_line( const char *progname ) {
  STDOUT("%-20s: announce job completions for long running jobs\n", progname );
  exit(0);
}
void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
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
      opt,
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
    }

    // Pre-Check
    if ( optarg ) {                   // only check if not null
      switch (opt) {                  // check only args with possible STRING options
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

  for (; optind < argc; optind++) {         // process remainder of cmdline using argv[optind]
    BUGOUT("%2d: %s\n", optind, argv[optind] );
    load_file( argv[optind] );
  }                                         // for optind

  exit(0);
}
