# The Twin Text Windows Library

"Twin" is a library for manipulating terminal text windows using
_xterm_ control/escape sequences.  It's inspired by earlier work in
the venerable _curses_ library, but it's not designed to be compatible
with it.  In particular it:

* is hardcoded for _xterm_, 256 colours
* supports multiple screens/terminals simultaneously
* composes line-graphics characters
* uses double buffering to optimise updates, with region damage management
* supports a hierarchy of terminal sub windows (not yet).

It doesn't support input handling yet, it's currently output only.
That makes it useful enough for dashboards etc. so far.

## Example

```c
#include <unistd.h>
#include <stdio.h>
#include <xterminator.h>

int main(int argc, char *argv[])
{
    static XterminatorPtr xterminator = new_xterminator(0, stdout);
    TwindowPtr root = &xterminator->root;

    open_xterminator(xterminator); /* initialise the terminal device */

    twin_box(root,                 /* draw a line-graphics box */
             0, 0,
             root->geometry.size.row, root->geometry.size.column);
    twin_cursor(root, 0, 1);
    twin_puts(root, "hello world"); /* print some text */
    xt_sync(xterminator);           /* ...now update the screen */
	sleep(10);

    close_xterminator(xterminator); /* reset the terminal device */
    free_xterminator(xterminator);
}
```
