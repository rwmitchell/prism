#include <stdio.h>
#include <sys/time.h>  // gettiemofday() -> NOW()

double NOW() {
  static struct timeval tv;
  gettimeofday( &tv, NULL );
  return( tv.tv_sec + tv.tv_usec*1e-6 );
}

static double start = 0.0;
void rtimer() {
  start = 0.0;
}
char *timer() {
  static char  str[64];  // give plenty of space in case delta time is large

  if ( start == 0.0 ) start = NOW();
  sprintf(str, "%7.3lf", NOW()-start );
  return( str );
}


