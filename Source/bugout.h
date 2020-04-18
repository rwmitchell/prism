#include <libgen.h>
#include <dirent.h>
#include <signal.h>

char __buf[MAXNAMLEN],           // make name hidden/unique
     *timer();

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
  kill( 0, SIGINT ); \
  exit( __LINE__ ); \
}
#define GTFERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fprintf(stderr, " -- Exiting\n" ); \
  fflush (stderr); \
  kill( 0, SIGINT ); \
  exit( __LINE__ ); \
}
#define TIMOUT( FMT, ... ) { \
  fprintf(stdout, "%s: ", timer() ); \
  fprintf(stdout, "%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}

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

