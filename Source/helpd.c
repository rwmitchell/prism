#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strncasestr()
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include "bugout.h"

void usage(struct option longopts[]) {
  bool B_zsh = false;

  if ( strcasestr( getenv("SHELL"), "zsh" ) != NULL ) B_zsh = true;

  for ( int i=0; longopts[i].name != NULL; ++i ) {
    STDOUT("%s%s%s%s%s%c ",
      B_zsh ? "-" : "",
      longopts[i].name,
      longopts[i].has_arg == optional_argument ? "=" : "",
      B_zsh ? "\n" : " ",
      B_zsh ? "-" : "",
      isprint(longopts[i].val) ? longopts[i].val : ' '
    );
    if ( B_zsh ) STDOUT("\n");
  }
  if ( !B_zsh ) STDOUT("\n");
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

  for ( int i=0; lopts[i].name != NULL; ++i ) {
    STDERR("%2d  -%c  %-9s %-15s\n",
      i,
      isprint(lopts[i].val) ? lopts[i].val : ' ',
      args[lopts[i].has_arg],
      lopts[i].name
    );
  }
  STDERR("\n");
}
