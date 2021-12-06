#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strcpy()
#include <getopt.h>
#include <ctype.h>      // isdigit()
#include <fcntl.h>      // open()
#include <sys/stat.h>   // fsize() / stat()
#include <stdbool.h>    // bool
#include <mylib.h>
#include "traveler.h"

// Load 1K of text into shmem
// This will be read by traveler.c
// and displayed as a "secret" messsage

const
char *cvsid = "$Id$";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0,
     lskp  =  1,
     wsec  =  0;

bool B_o    = false,
     B_rmid = false;

/*
"@(#)$Id$";
*/
#ifdef  NO_loadfile
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

  if ( ! file_exists( fname ) ) {
    BUGOUT("%s does not exist, exiting\n", fname );
    exit( __LINE__ );
  }

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
#endif
void help( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n\n", cvsid);
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("-o=STRING     (%s : %s)\n", TF[B_o], myopt );
  STDERR("-d INTEGER    (%d)\n", debug );
  STDERR("try again later\n");
  STDERR("\n");

  if ( debug ) RMhelpd( lopts );

  exit(-0);
}

int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt,
      longindex;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":Xl:o:Rw:d:uh";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "example", no_argument,       NULL, 'X' },
    { "myopt",   optional_argument, NULL, 'o' },
    { "lskip",   required_argument, NULL, 'l' },
    { "wait",    required_argument, NULL, 'w' },
    { "reset",   no_argument,       NULL, 'R' },
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

      case 'l': lskp  = strtol( optarg, NULL, 10 );
                break;

      case 'w': wsec  = strtol( optarg, NULL, 10 );
                break;

      case 'R': B_rmid = !B_rmid; break;

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

      case 'h':
      default :
        ++errflg;
        break;
    }
  }

  if (errflg) help(argv[0], opts, longopts);


  // Setup SHMEM
#define DO_SHMEM
#ifdef  DO_SHMEM
  bool shm_exists = false;
  int  shmid_secret;
  TRAVELER_t *block;
  size_t      block_sz;
  char       *data,
             *pt;
  off_t       f_sz;


  shm_exists = RMcheck_shmem( KEY_TRAVELER,  &shmid_secret );
//BUGOUT("SHMEM %s: %d\n", shm_exists ? "Exists" : "------", shmid_secret);

  block_sz = sizeof( TRAVELER_t );
  block    = (TRAVELER_t *) RMsetup_shmem( !shm_exists, KEY_TRAVELER, block_sz, &shmid_secret);

  if ( B_rmid ) {
    BUGOUT( "Releasing shared memory");
    shmctl(shmid_secret, IPC_RMID, (struct shmid_ds *) block);
    STDOUT( " - shared memory has been released\n");
    exit(0);
  }

#endif

  if ( optind+1 == argc && RMfile_exists( argv[optind] ) ) {
    data = (char *) RMloadfile( argv[optind], &f_sz, false );
  } else {

//if ( optind+1 < argc ) {     // if multiple args, assume it is the message
    int i;

    f_sz = argc+4;
    for ( i=optind; i<argc; ++i ) f_sz += strlen( argv[i] );
    data = (char *) malloc( f_sz );
    strcat( data, "| " );
    for ( i=optind; i<argc; ++i ) {
      strcat( data, argv[i] );
      strcat( data, " " );
    }
    strcat( data, "|\n" );
  }


  if ( block->time == 0.0 ) {
      memset( block->text, ' ', MAX_SECRET );
      pt = block->text;
  } else {
    pt = strrchr( block->text, '\n' );
    pt++;
  }
  if ( pt - block->text + f_sz < MAX_SECRET ) {
      memcpy( pt, data, MAX_SECRET);
      block->time = RMNOW() + wsec;
      block->lskp = lskp;
  }
  STDOUT("%lf\n", block->time );
  STDOUT("%s\n", block->text );
  free( data );

  exit(0);
}
