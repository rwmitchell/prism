#include <stdio.h>
#include <stdlib.h>    // strtol()
#include <unistd.h>    // access()
#include <string.h>    // strncpy()
#include <strings.h>   // rindex()
#include <sys/stat.h>  // fstat()
#include <dirent.h>    // MAXNAMLEN
#include <stdbool.h>
#include "strings.h"   // truncname
#include "types.h"

char *rindex(const char *s, int c);

off_t  fsize( const char *name ) {       // return size of the named file
  off_t size = -1L;
  struct stat sbuf;

  if (stat(name,&sbuf) == 0)
    if ( (sbuf.st_mode & S_IFMT) == S_IFREG)
      size=sbuf.st_size;

  return(size);
}
off_t fdsize( int fd ) {           // return size of fd
  off_t size = -1L;
  struct stat buf;

  if ( fstat( fd, &buf ) < 0 )
    perror( __func__ );
  else
    size = buf.st_size;

  return( size );
}

bool exists( const char *path, const char *name ) {
  bool rv = false;
  char tmp[MAXNAMLEN];
  strncpy( tmp, path, MAXNAMLEN );
#ifdef __APPLE__
  strncat( tmp, "/", 1);
#else
  strcat( tmp, "/" );
#endif
  strncat( tmp, name, MAXNAMLEN-(strlen(tmp) + strlen(name)) );

  if ( !access( tmp, R_OK ) ) rv = true;

#ifdef  DEBUG
  if ( debug & 0x0100 )
    printf("%s:%d: tmp: %s\n", __func__, __LINE__, tmp );
#endif

  return( rv );
}
int file_exists( const char *filename ) {
  struct stat buf;
  return( stat(filename, &buf) == 0 );
}
char *findfile( const char *path, const char *name ) {
  bool found = false,
       error = false;
  static char tmp[MAXNAMLEN];

  strncpy(tmp, path, MAXNAMLEN);
  while ( !found && !error ) {
    found = exists( tmp, name );
    if ( ! found && strlen(tmp) > 0 ) {
      truncname( tmp, tmp, '/' );
#ifdef  DEBUG
      if ( debug & 0x0100 )
        printf("%s:%d: path: %s\n", __func__, __LINE__, tmp );
#endif
    }
    error = strlen(tmp) <= 0 ;
  }
#ifdef __APPLE__
  strncat( tmp, "/", 1);
#else
  strcat( tmp, "/" );
#endif
  strncat( tmp, name, MAXNAMLEN-(strlen(tmp) + strlen(name)) );

  return( error ? (char *) NULL : tmp );
}
int change_ext( const char *in, char *out, const char *old_ext, const char *new_ext ) {
  int ret = 0;
  char *pc;

  strcpy( out, in );

  if ( ( pc = strstr(out, old_ext)) == NULL ) {
    // fprintf(stderr, "did not find %s\n", old_ext );
    pc = out;
  } else {
    *pc = '\0';
  }
  strcat(pc, new_ext );

  return( ret );
}
int mkpath( const char *fullpath, mode_t mode ) {
  char oname[MAXNAMLEN],
       nname[MAXNAMLEN],
      *ptr, *ptr2;
  mode_t mask = 000, omask;
#ifdef arm
  struct group *grp;
#endif
  gid_t eaarl_gid = 700;
  // extern int errno;   // not all systems liked this.

  omask = umask(mask);  // when run as a daemon, owner is root/root

  // printf("mkpath: changed mask from %04o to %04o\n", omask, mask);

  strncpy(oname, fullpath, MAXNAMLEN);
  ptr2 = rindex(oname, '/');
  *ptr2 = '\0';

  if ( mode == 0 ) mode = 0770;  // my default setting

  ptr = strtok(oname, "/");
  nname[0] = '\0';
#ifdef arm
  grp = getgrnam("eaarl");
  if ( grp != (struct group *) NULL ) eaarl_gid = grp->gr_gid;
  else fprintf(stdout, "mkpath: getgrnam failed, setting eaarl gid to %d\n", eaarl_gid);
#else
  // Something has broken this when compiling under vm1 ubuntu 9.04 and
  // running on ubuntu 10.04.  just hard code the value we want
  eaarl_gid = 700;
#endif

  while ( ptr != (char *) NULL) {
    sprintf(nname, "%s/%s", nname, ptr);
    if ( access(nname, R_OK) == -1 ) {  // doesn't exist, must make
      mkdir(nname, mode);
    }
    // errno = 0;   // not all systems liked this.
    if ( chown(nname, -1, eaarl_gid) == -1 ) {
      perror( "mkpath: chown: " );
      fprintf(stdout, "mkpath: chown: %s FAILED\n", nname);
    }        // set group to eaarl
    ptr = strtok((char *) NULL, "/");
  }

  return(0);
}
/* fgetl:  get line into s from fin, return length
 *         this was created from getline() on pg 69 in K&&R
 *         "The C Programming Language, second edition".
*/
int fgetl( FILE *fin, char *s, int lim) {
  int c, i=0;

  while (--lim >= 0 && (c=fgetc(fin)) != EOF && c != '\n') {
    s[i++] = c;
  }
  if ( c == '\n' ) s[i++] = c;
  s[i] = '\0';
  if ( c == EOF  )i = -1;
  return(i);
}
