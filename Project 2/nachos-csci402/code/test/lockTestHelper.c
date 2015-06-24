/* lockTestHelper.c
 *	Simple program to help with the lockTest.
 */

#include "syscall.h"

int
main()
{
    /* Try to acquire, release, and destroy lock #2; shouldn't work */
    Acquire(2);
    Release(2);
    DestroyLock(2);
}
