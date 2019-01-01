/* io.c */
off_t fsize(const char *name);
off_t fdsize(int fd);
char *findfile(const char *path, const char *name);
int change_ext(const char *in, char *out, const char *old_ext, const char *new_ext);
int mkpath(const char *fullpath, mode_t mode);
int fgetl(FILE *fin, char *s, int lim);
