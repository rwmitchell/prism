/*******************************************************************
 *    $Id$
 *    Generic routine to open connection to shared memory
*******************************************************************/

#include <stdio.h>
#include <stdlib.h>     // exit
#include <string.h>     // memset
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/param.h>  // MAXPATHLEN
#include <sys/socket.h> // UDP
#include <stdbool.h>
#include "shmem.h"

enum {
  bufsz  = 128,
  LOGLEN = 1024,
};

// These should match UDP_TYPES in udp.h
// This is used to define the JSON record name and is used by:
// upd_linkth.c, pwr_json.c, cpt json_rout.c, udp_laser.c.
// info uses info.conf to populate SFT where the names can
// be changed without recompiling - for something more
// user friendly
const char *udp_names[] = {
  "UNKN",           //  0  - this does NOT get broadcast
  "TRAVELER",       //  1
  "EOT",            //  2
  "MAX_UDP",        //  3
};

// **********************************************

extern int errno;

void fail( const char *s ) {

  puts(s);
  perror(s);

  exit(1);
}

static int get_size( key_t key ) {
  size_t size = 0;
  switch (key) {

//  case KEY_TARGET    : size = sizeof( SomeTypedef_t ); break;

    default: break;
  }

  return(size);
}

// *** Check status of generic shared memory segment
bool  check_shmem( key_t key, int *id ) {
  char *pmem = NULL;
  size_t size;

  size = get_size(key);
  // printf("KEY = %0x, size = %d\n", key, size);
  *id = shmget( key, size, 0666 );
  if ( *id < 0 ) {              // Doesn't exist
  } else {
    pmem = (char *)shmat( *id, NULL, 0 );
  }
  return( (pmem == NULL ) ? false : true );
}


// *** Open generic shared memory segment
char  *setup_shmem( bool create, key_t key, size_t size, int *id ) {
  char *pmem;

  *id = shmget(key, size, 0666);

  if ( create && *id < 0 ) {    // Doesn't exist, create it.
    *id = shmget(key, size, IPC_CREAT | 0666);
    if (*id < 0 ) {
       fail("\ncan't allocate shared memory");
    }
  } else {
    if ( *id < 0 )
      fail ("\ncan't find required shared memory segment");
  }

// *** Attach the shared memory to pmem
  pmem = (char *)shmat( *id, NULL, 0 );
  if ( (int)*pmem  == -1 ) {
     fail("can't attach to shared memory");
  }

  return( pmem );
}
