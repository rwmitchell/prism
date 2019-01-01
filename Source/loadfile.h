/* Source/loadfile.c */
void *mymalloc( void *pointer, size_t size );
int str2arr(char *mlstr, const char *FS, char ***arr, int lim);
unsigned char *loadfile(const char *fname, off_t *f_sz);
unsigned char *loadstdin( off_t *f_sz );
