#include <libgen.h>
#include <dirent.h>
char buf[MAXNAMLEN];
#define BUGOUT( FMT, ... ) { \
  fprintf(stdout, "%s: %5d:%-24s:", basename_r(__FILE__, buf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define BUGERR( FMT, ... ) { \
  fprintf(stderr, "%s: %5d:%-24s:", basename(__FILE__), __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
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
char *timer();
#define TIMOUT( FMT, ... ) { \
  fprintf(stdout, "%s: ", timer() ); \
  fprintf(stdout, "%s: %5d:%-24s:", basename(__FILE__), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}

