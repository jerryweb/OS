/* lockTest.c
 *	Simple program to test whether the Lock syscalls work with a single thread.
 */

#include "syscall.h"
int a[3];
int b, c;

int goodLockID, badLockID;

void LockThreadTest1()
{
    Printf("Start thread 1\n", 13, 0, 0);
    Acquire(goodLockID);
    Yield();
    Release(goodLockID);
    Exit(0);
}
void LockThreadTest2()
{
    Printf("Start thread 2\n", 13, 0, 0);
    Acquire(goodLockID);
    Yield();
    Release(goodLockID);
    Exit(0);
}

int
main()
{
    Printf("Testing lock ID\n", 16, 0, 0);
    /* Create with bad name string */
    badLockID = CreateLock((char*)0, -1);
    Printf("Lock ID = %d\n", 13, 1, badLockID);
    /* Create with good name string */
    goodLockID = CreateLock("Test1", 5);
    Printf("Lock ID = %d\n", 13, 1, goodLockID);
    /* Acquire bad lock IDs */
    Acquire(badLockID);
    Acquire(goodLockID + 1);
    /* Acquire good lock ID */
    Acquire(goodLockID);
    /* Release good lock ID */
    Release(goodLockID);
    /* Release bad lock IDs */
    Release(goodLockID + 1);
    Release(badLockID);
    /* Destroy bad lock IDs */
    DestroyLock(badLockID);
    DestroyLock(goodLockID + 1);
    /* Destroy good lock ID */
    DestroyLock(goodLockID);
    Printf("Testing locks on multiple threads\n", 34, 0, 0);
    goodLockID = CreateLock("Test2", 5);
    Printf("Lock ID = %d\n", 13, 1, goodLockID);
    Fork(LockThreadTest1, "thread1", 7);
    Fork(LockThreadTest2, "thread2", 7);
    Yield();
    Printf("Testing locks on multiple processes\n", 36, 0, 0);
}
