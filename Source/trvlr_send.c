#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strcpy()
#include <getopt.h>
#include <ctype.h>      // isdigit()
#include <fcntl.h>      // open()
#include <sys/stat.h>   // fsize() / stat()
#include <stdbool.h>    // bool
#include "bugout.h"
#include "shmem.h"
#include "traveler.h"
#include "now.h"

// Load 1K of text into shmem
// This will be read by traveler.c
// and displayed as a "secret" messsage

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0;

bool B_o = false;

/*
"@(#)$Id$";
*/

off_t fsize( char *name ) {
  off_t size = -1L;
  struct stat sbuf;

  if (stat(name,&sbuf) == 0)
    if ( (sbuf.st_mode & S_IFMT) == S_IFREG)
      size=sbuf.st_size;

  return(size);
}
char *loadfile( char *fname, off_t *f_sz ) {
  off_t   f_limit = 32 * 2<<19;            // ~ 32MB, arbitrary
  ssize_t rc;
  int     fd;
  static
  char *data = NULL;

  *f_sz = fsize( fname )+1;                // get file size
  if ( debug & 0x0001 )
    BUGOUT( "%12llu file size\n", *f_sz );

  if ( *f_sz > 0 && *f_sz < f_limit ) {    // is file size in bounds?
    data = (char *) malloc( *f_sz );       // allocate space for file
    memset(data, '\0', *f_sz );

    if ( debug & 0x0001 )
    BUGOUT( "%p data: bytes: %llu\n", data, *f_sz );

    if ( (fd=open( fname, O_RDONLY )) > 0 ) {
      rc = read( fd, data, *f_sz );        // put entire file into data
      close( fd );
    } else {
      BUGERR( "Unable to open %s, %d\n", fname, fd );
    }
  } else {
    BUGERR( "file size: %llu   limit = %llu\n", *f_sz, f_limit );  // size out of bounds
  }

  if ( debug & 0x0002 ) {
    BUGOUT( "File Contents\n" );
    STDOUT( "--------------\n" );
    STDOUT( "%s", data );
    STDOUT( "--------------\n" );
  }

  return( data );
}
void usage(struct option longopts[]) {
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
void help( char *progname, const char *opt, struct option lopts[] ) {
  int i;
  const char *args[] = {"None", "Required", "Optional"};

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("-o=STRING     (%s : %s)\n", TF[B_o], myopt );
  STDERR("-d INTEGER    (%d)\n", debug );
  STDERR("try again later\n");
  STDERR("\n");

  if ( debug ) {
    STDERR("\n");
    STDERR("%2s %-15s %-9s %4s\n",
        "  ",
        "Name",
        "arg",
        "opt" );
    for (i=0; lopts[i].name != NULL; ++i ) {
      STDERR("%2d %-15s %-9s   %c",
        i,
        lopts[i].name,
        args[lopts[i].has_arg],
        isprint(lopts[i].val) ? lopts[i].val : ' '
      );
      if ( lopts[i].flag != NULL ) {
        STDERR(" Pointer: %d", *lopts[i].flag );
      }
      STDERR("\n");
    }
  }

  exit(-0);
}

int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt, i,
      longindex;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":Xo:d:uh";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "example", no_argument,       NULL, 'X' },
    { "myopt",   optional_argument, NULL, 'o' },
    { "debug",   optional_argument, NULL, 'd' },
    { "help",    no_argument,       NULL, 'h' },
    { "usage",   no_argument,       NULL, 'u' },
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

      case 'h':
      default :
        ++errflg;
        break;
    }
  }

  if (errflg) help(argv[0], opts, longopts);

  BUGOUT("Args already processed:\n");
  for ( i=0; i<optind; ++i )
    BUGOUT("%d: %s\n", i, argv[i] );

  BUGOUT("debug level: %d\n", debug );
  BUGOUT("      myopt: %s\n", myopt );

  // Setup SHMEM
#define DO_SHMEM
#ifdef  DO_SHMEM
  bool shm_exists = false;
  int  shmid_secret;
  TRAVELER_h *block;
  size_t      block_sz;
  char       *data;
  off_t       f_sz;

  block_sz = sizeof( TRAVELER_h );
  block    = (TRAVELER_h *) setup_shmem( !shm_exists, KEY_TRAVELER, block_sz, &shmid_secret);

  shm_exists = check_shmem( KEY_TRAVELER,  &shmid_secret );
  BUGOUT("SHMEM %s: %d\n", shm_exists ? "Exists" : "------", shmid_secret);
#endif

//for (; optind < argc; optind++) {}        // process remainder of cmdline using argv[optind]

    BUGOUT("%2d: %s\n", optind, argv[optind] );
    data = loadfile( argv[optind], &f_sz );
    BUGOUT("f_sz: %lld\n", f_sz );
    STDOUT("%s\n", data );
    memset( block->text, ' ', MAX_SECRET );
    memcpy( block->text, data, MAX_SECRET);
    block->time = NOW();
    STDOUT("-------\n" );
    STDOUT("%lf\n", block->time );
    STDOUT("%s\n",  block->text );

    free( data );

  exit(0);
}
