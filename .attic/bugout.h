#include <libgen.h>
#include <dirent.h>
#include <signal.h>
#include <stdbool.h>

#ifndef BUGOUT_H
#define BUGOUT_H

extern
bool _itwasme; // = false;

#define _FCL const char *F, const char *C, int L

char __dbuf[MAXNAMLEN],           // make name hidden/unique
     *timer();
#ifdef  __APPLE__
#define BUGOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define BUGERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
#define GTFOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fprintf(stdout, " -- Exiting\n" ); \
  fflush (stdout); \
  _itwasme = true; \
  kill( 0, SIGCONT ); \
  exit( __LINE__ ); \
}
#define GTFERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fprintf(stderr, " -- Exiting\n" ); \
  fflush (stderr); \
  _itwasme = true; \
  kill( 0, SIGUSR1 ); \
  exit( __LINE__ ); \
}
#define TIMOUT( FMT, ... ) { \
  fprintf(stdout, "%s: ", timer() ); \
  fprintf(stdout, "%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define FCLOUT( F, C, L, FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s: ", basename_r( F, __dbuf), L, C ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define FCLERR( F, C, L, FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s: ", basename_r( F, __dbuf), L, C ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
#else
#define BUGOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define BUGERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
#define GTFOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fprintf(stdout, " -- Exiting\n" ); \
  fflush (stdout); \
  _itwasme = true; \
  kill( 0, SIGUSR1 ); \
  exit( __LINE__ ); \
}
#define GTFERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fprintf(stderr, " -- Exiting\n" ); \
  fflush (stderr); \
  _itwasme = true; \
  kill( 0, SIGUSR1 ); \
  exit( __LINE__ ); \
}
#define TIMOUT( FMT, ... ) { \
  fprintf(stdout, "%s: ", timer() ); \
  fprintf(stdout, "%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#endif
#define BUGNUL( FMT, ... ) { \
}                // Do nothing
#define STDOUT( FMT, ... ) { \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
}
//fflush (stdout); \  // Removed STDOUT, big performance hit!
#define STDERR( FMT, ... ) { \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
}
#define STDNUL( FMT, ... ) { \
}                // Do nothing
#endif
