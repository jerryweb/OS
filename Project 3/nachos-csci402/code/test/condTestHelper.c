/* condTestHelper.c
 *	Simple program to help with condTest.
 */

#include "syscall.h"

int
main()
{
    /* Try syscalls on cond #1; shouldn't work */
    Wait(1, 0);
    Signal(1, 0);
    Broadcast(1, 0);
    DestroyCondition(1);
}
