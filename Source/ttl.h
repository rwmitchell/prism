#ifndef TTL_H
#define TTL_H

typedef struct {           // STR_t
  int   len,
        siz;
  char *str;
} STR_t;
typedef struct {           // TTL_ctl_h
  int  ncmds;
  char *cmds;       // |cmd|cmd|cmd|
  int  *noc;        // number of counts for each command
  int  *pos;        // start position   for each command
} TTL_ctl_h;
typedef struct {           // TTL_msg_h
  int   len;
  char *msg;
  int  *arg;
} TTL_msg_h;

#endif
