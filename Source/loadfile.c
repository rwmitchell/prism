/* loadfile.c
 * prototype code loading an entire file with a single read
 * and then converting to array of strings
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strcpy()
#include <getopt.h>
#include <sys/stat.h>   // fsize() / stat()
#include <fcntl.h>      // open()
#include <sys/types.h>  // read()
#include <sys/uio.h>    // read()
#include <unistd.h>     // read()
#include <sys/param.h>  // str2arr(), INT_MAX
#include <stdbool.h>
#include <sys/param.h>  // MIN/MAX
#include "io.h"
#include "bugout.h"

extern int  debug;

void *mymalloc( void *pointer, size_t size ) {    // copied from atm_hdf5.c
  if ( pointer != NULL ) free( pointer );

  pointer =  malloc( size );
  if ( pointer == NULL ) {
    BUGOUT("Failed to allocate %lu bytes, exiting\n", size );  // go to any user redirection
    BUGERR("Failed to allocate %lu bytes, exiting\n", size );  // go to the screen
    exit(0);                // no need to check return value for NULL, we've already exited
  }
  memset( pointer, '\0', size );
  return( pointer );
}

// ###############################################################
int   count_lines  ( char  *fname ) {                 // count lines in file
  int rv = 0,
      ch;
  FILE *fp;

  if ( (fp=fopen(fname, "r")) == NULL ) {
    BUGERR("Unable to open: %s\n", fname );
    exit( __LINE__ );
  }

  while ( (ch=fgetc(fp)) != EOF ) if ( ch == '\n' ) ++rv;
  fclose( fp );

  return( rv );
}
int   max_linelen( char **arr ) {
  char **pa = arr;
  size_t  max=0;
  while ( **pa ) {
//  STDOUT("%3lu: %s\n", strlen( *pa ), *pa );
    max = MAX( max, strlen( *pa ) );
    pa++;
  }
//STDOUT("%3lu: MANLEN\n", max );

  return( max );
}
int   str2arr( char *mlstr, const char *FS, char ***arr, int lim ) {
  char *ch;
  int i, cnt, pos;

  if ( lim < 0 ) lim = INT_MAX;

  // count the number of FS in the string
  for (cnt=0, ch=mlstr; *ch != '\0' && cnt <= lim; ++ch )
    if ( strchr( FS, *ch ) != NULL ) cnt++;
  cnt++;

//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

  if ( cnt > lim ) cnt = lim;

  *arr = (char **) malloc( sizeof(char *) * cnt );   // alloc space for array of pointers, mymalloc() fails here

  ch = mlstr;
  for (i=0, cnt=0, pos=0; *ch != '\0' && cnt < lim-1; ++i, ++ch ) {
    if ( strchr( FS, *ch ) != NULL ) {
      *ch = '\0';
      if ( i-pos > 1 ) {
//      BUGOUT("%2d:%d:%5d: >%s<\n", cnt, i, pos, &mlstr[pos] );
        (*arr)[cnt++] = &mlstr[pos];
      }
      pos = i+1;
    }
  }
  (*arr)[cnt++] = &mlstr[pos];   // save the last one
//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

#ifdef __APPLE__off
  BUGOUT("malloc_size(*arr) = %lu\n", malloc_size( *arr ) );
#endif

  return( cnt );
}
// ---------------------------------------------------------------
unsigned char *loadfile( const char *fname, off_t *f_sz ) {
  off_t   f_limit = 384 * 2<<19;            // ~ 384, arbitrary
  ssize_t rc = 0;
  int     fd;
  static
  unsigned char *data = NULL;

  if ( ! file_exists( fname ) ) {
    BUGOUT("%s does not exist, exiting\n", fname );
    exit( __LINE__ );
  }
  
  *f_sz = fsize( fname )+0;                // get file size
  if ( debug & 0x0001 )
    BUGOUT( "%12lld file size\n", *f_sz );

  if ( *f_sz > 0 /* && *f_sz < f_limit */ ) {    // is file size in bounds?
    *f_sz = MIN( *f_sz, f_limit );
    data = (unsigned char *) mymalloc( data, *f_sz+1 );       // allocate space for file
//  memset(data, '\0', *f_sz+1 );

    if ( debug & 0x0001 )
    BUGOUT( "%p data: bytes: %lld\n", data, *f_sz );

    if ( (fd=open( fname, O_RDONLY )) > 0 ) {
      rc = read( fd, data, *f_sz );        // put entire file into data
      close( fd );
    } else {
      BUGERR( "Unable to open %s, %d\n", fname, fd );
    }
  } else {
    BUGERR( "file size: %lld   limit = %llu\n", *f_sz, f_limit );  // size out of bounds
  }

  if ( rc != *f_sz )
    BUGOUT("Read %ld bytes of %s, expected %lld\n", rc, fname, *f_sz );

#ifdef  NOT_HELPFUL
  if ( debug & 0x0002 ) {
    BUGOUT( "File Contents\n" );
    STDOUT( "--------------\n" );
    STDOUT( "%s", data );
    STDOUT( "--------------\n" );
  }
#endif

  return( data );
}

// ###############################################################
#define BUF_SIZE 1024
#define BUF_MIN   256
unsigned char *loadstdin( off_t *f_sz ) {
  static
  char *input = NULL;
  char *pi;
  size_t sz;
  off_t  len = 0,
         size;

  if ( input ) free( input );

  size   = BUF_SIZE;
  input  = (char *) mymalloc( input, size );

  pi = input;
  while ( ! feof( stdin ) ) {
    if ( size-len <= BUF_MIN ) {
      size   *= 2;

#ifdef __APPLE__
      input = (char *) reallocf( input, size );
#else
      input = (char *) realloc ( input, size );
#endif
      if ( !input ) *f_sz = -1;
      pi = input + len;
    }

    sz = fread( (void *) (pi), 1, size-len, stdin );
    pi += sz;

    len += sz;
  }

  *f_sz = len;

  freopen("/dev/tty", "r", stdin );
//BUGOUT("LEN: %lld\n", len )
  return( (unsigned char *) input );
}
