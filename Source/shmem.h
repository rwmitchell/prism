// $Id$
#include <stdbool.h>
#include <sys/shm.h>    // size_t

enum {                     // SHMEM Keys
  KEY_TRAVELER = 0x40001,  // 196609d  contents of Target.cfg - not used.
};

/* Source/shmem.c */
void alert(const char *s);
void fail(const char *s);
void blog(char *s);
bool check_shmem(key_t key, int *id);
char *setup_shmem(bool create, key_t key, size_t size, int *id);
void diep(const char *s);

