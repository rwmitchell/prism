// $Id$

#include <stdio.h>
#include <stdbool.h>

#define  I8  char
#define UI8  unsigned I8
#define  I16 short
#define UI16 unsigned I16
#define  I32 int
#define UI32 unsigned I32
#define  I64 long long
#define UI64 unsigned I64

typedef unsigned char uchar;

FILE *save_to( char *dir, char *ext, bool gzip, char *fname);
FILE *daily_log( char *dir, char *ext, char *fname);
