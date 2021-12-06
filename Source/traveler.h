
enum {
  MAX_SECRET   = 1024,
  KEY_TRAVELER = 0x40001,
};
typedef struct {
  double time;             // time to play msg
  int    lskp;             // lines to skip
  char   text[MAX_SECRET];
} TRAVELER_t;
