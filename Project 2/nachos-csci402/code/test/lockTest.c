/* lockTest.c
 *	Simple program to test whether the Lock syscalls work with a single thread.
 */

#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int goodLockID, badLockID;
    /* Create with bad name string: ID -1 */
    badLockID = CreateLock((char*)0, -1);
    Printf("Lock ID = %d\n", 13, 1, badLockID);
    /* Create with good name string */
    goodLockID = CreateLock("Test", 4);
    Printf("Lock ID = %d\n", 13, 1, goodLockID);
    /* Acquire bad lock IDs - will print errors */
    Acquire(badLockID);
    Acquire(goodLockID + 1);
    /* Acquire good lock ID */
    Acquire(goodLockID);
    /* Release good lock ID */
    Release(goodLockID);
    /* Release bad lock IDs - will print errors */
    Release(goodLockID + 1);
    Release(badLockID);
    /* Destroy bad lock IDs - will print errors */
    DestroyLock(badLockID);
    DestroyLock(goodLockID + 1);
    /* Destroy good lock ID */
    DestroyLock(goodLockID);
}
