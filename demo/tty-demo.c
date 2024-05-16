/*
 * https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

struct termios orig_tty;

void tty_restore(int fd, struct termios *tty_config)
{
    tcsetattr(fd, TCSAFLUSH, tty_config);
}

void tty_exit(void)
{
    tty_restore(0, &orig_tty);
}

void tty_raw(int fd)
{
    struct termios raw_tty;
    tcgetattr(fd, &raw_tty);

    raw_tty.c_iflag &= (tcflag_t) ~(
        BRKINT                  /* disable ^C? */
        | ICRNL                 /* disable LF -> CR-LF mapping on input */
        | INPCK                 /* disable parity checking? */
        | ISTRIP                /* disable 7-bit stripping */
        | IXON);                /* disable software flow control */
    raw_tty.c_oflag &= (tcflag_t) ~(
        OPOST);            	/* disable LF -> CR-LF mapping on output */
    raw_tty.c_cflag |= (CS8);   /* characters are 8 bits */
    raw_tty.c_lflag &= (tcflag_t) ~(
        ECHO                    /* disable echo */
        | ICANON         	/* disable canonical mode (line buffering) */
        | IEXTEN         	/* disable ^V paste buffer */
        | ISIG);         	/* disable ^C, ^Z signal mapping */
    raw_tty.c_cc[VMIN] = 0;     /* minimum No. bytes to read */
    raw_tty.c_cc[VTIME] = 10;    /* read wait time ( x 0.1s) */
    tcsetattr(fd, TCSAFLUSH, &raw_tty);
}

int main(void)
{
    tcgetattr(STDIN_FILENO, &orig_tty);
    atexit(tty_exit);

    tty_raw(STDIN_FILENO);
    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
        {
            // die("read");
            exit(1);
        }
        printf("char %d: ", c);
        if (iscntrl(c))
        {
            printf("(^%c)\r\n", c + '@');
        } else
        {
            printf("%c\r\n", c);
        }
        if (c == 'q') break;
    }
    return 0;
}
