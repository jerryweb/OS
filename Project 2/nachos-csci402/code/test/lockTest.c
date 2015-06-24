/* lockTest.c
 *	Simple program to test whether Lock syscalls work.
 *  Runs through every possible case of every syscall.
 *  MUST BE RUN WITHOUT -RS TO ENSURE PROPER SEQUENCE
 */

#include "syscall.h"

int goodLockID, badLockID;

void LockThreadTest1()
{
    /* Acquire the lock, then yield back to main thread */
    Acquire(goodLockID);
    Yield();
    /* Run "critical section", then release/destroy/exit - this should happen before main's "critical section", but the lock shouldn't be deleted until main releases */
    Printf("thread1 critical section\n", 25, 0, 0);
    Release(goodLockID);
    DestroyLock(goodLockID);
    Exit(0);
}

int
main()
{
    /* PART 1: TEST ID FAILURE CONDITIONS */
    Printf("Testing lock ID failure conditions\n", 35, 0, 0);
    /* Create with bad name string; ID should always be -1 */
    badLockID = CreateLock((char*)0, -1);
    Printf("Lock ID = %d\n", 13, 1, badLockID);
    /* Create with good name string; ID should always be 0 */
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
    /* Test all syscalls with the now-deleted lock */
    Acquire(goodLockID);
    Release(goodLockID);
    DestroyLock(goodLockID);
    /* PART 2: TEST MULTITHREADED LOCKS */
    Printf("Testing locks on multiple threads\n", 34, 0, 0);
    /* Create a new lock, since the old one was deleted; ID should always be 1 */
    goodLockID = CreateLock("Test2", 5);
    Printf("Lock ID = %d\n", 13, 1, goodLockID);
    /* Fork a new thread, then let it run immediately */
    Fork(LockThreadTest1, "thread1", 7);
    Yield();
    /* Try to acquire the lock; won't finish until thread1 runs its "critical section" and does release/destroy/exit */
    Acquire(goodLockID);
    /* Run "critical section", then release and destroy for good */
    Printf("main critical section\n", 22, 0, 0);
    Release(goodLockID);
    DestroyLock(goodLockID);
    /* PART 3: TEST CROSS-PROCESS LOCK ACCESS */
    Printf("Testing locks on multiple processes\n", 36, 0, 0);
    /* Create a new lock, since the old one was deleted; ID should always be 2 */
    goodLockID = CreateLock("Test3", 5);
    Printf("Lock ID = %d\n", 13, 1, goodLockID);
    /* Exec a new process, then let it run immediately */
    Exec("../test/lockTestHelper", 22);
    Yield();
    DestroyLock(goodLockID);
}
