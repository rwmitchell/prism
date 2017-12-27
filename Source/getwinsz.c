// From:
// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

// Reports the size of the terminal if run directly,
// but cannot be run inside backticks or by another program

int main (int argc, char **argv)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    if ( argc > 1 ) {
      printf ( "lines %d\n", w.ws_row );
      printf ( "columns %d\n", w.ws_col );
    } else printf( "%d %d\n", w.ws_row, w.ws_col );

    return 0;  // make sure your main returns int
}

