/* printfTest.c
 *	Simple program to test whether the Printf syscall works.
 */

#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    /* Invalid strings  - should print 1 error */
    Printf((char*)10, 10, 0, 0);
    /* Invalid length - should not print anything */
    Printf("Hi\n", -1, 0, 0);
    /* Invalid number of parameters - should print 2 errors */
    Printf("Hi\n", 3, -1, 0);
    Printf("Hi\n", 3, 10, 0);
    /* Correct calls:
        Hi
        Hi 1
        Hi 1 2
        Hi 10 11 12
        Hi 23 45 67 89
    */
    Printf("Hi\n", 3, 0, 0);
    Printf("Hi %d\n", 6, 1, 101);
    Printf("Hi %d %d\n", 9, 2, 102);
    Printf("Hi %d %d %d\n", 12, 3, 101112);
    Printf("Hi %d %d %d %d\n", 15, 4, 123456789);
}
