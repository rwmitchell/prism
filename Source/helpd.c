#include <ctype.h>
#include <getopt.h>
#include "bugout.h"

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
void helpd( struct option lopts[] ) {
  const char *args[] = {"", "Required", "Optional"};

  STDERR("%2s  %3s %-9s %-15s\n",
      " #",
      "opt",
      "arg",
      "longname"
      );
  STDERR("%2s  %3s %-9s %-15s\n",
      "--",
      "---",
      "---------",
      "--------"
      );

  for (int i=0; lopts[i].name != NULL; ++i ) {
    STDERR("%2d  -%c  %-9s %-15s\n",
      i,
      isprint(lopts[i].val) ? lopts[i].val : ' ',
      args[lopts[i].has_arg],
      lopts[i].name
    );
  }
  STDERR("\n");
}
