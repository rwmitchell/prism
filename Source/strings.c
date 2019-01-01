#include <stdio.h>
#include <string.h>
#include <ctype.h>   // isspace()

char *adjustl( char *str, int len ) {     // move leading whitespace to the right side
  char *hd,  // head
       *cp;  // current position
  int  i,
       cnt = 0;

  hd = cp = str;
  while ( isspace( *str ) && cnt++ < len ) str++; // skip whitespace
  for (i=cnt; i<len; ++i ) *(cp++) = *(str++);    // copy
  for (i=0; i<cnt; ++i) *(cp++) = ' ';            // append space
  return( hd );
}
char *rtrim( char *str, int len ) {       // trim whitespace from right side
  char *cp;  // current position

  int  i,
       cnt = 0;

  --len;
  cp = str+len;

  while ( isspace( *cp ) && cnt++ < len ) cp--;   // skip whitespace
  *(++cp) = '\0';

  printf("%s:%d: cnt = %d\n", __func__, __LINE__, cnt );
  return( str );
}
// sorta copied from basename.c but doesn't overwrite in
char *truncname( char *in, char *out, char sep ) {
  char *period, *cin, *cout;

  period = strrchr( in, sep );
  if ( !period ) {
    strcpy( out, in );
  } else {
    for (cin=in, cout= out; cin != period; ++cin, ++cout ) *cout = *cin;
    *cout = '\0';
  }
  return( out );
}

