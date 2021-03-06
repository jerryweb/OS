/* lockTest.c
 *	Simple program to test whether Lock syscalls work.
 *  Runs through every possible case of every syscall.
 *  MUST BE RUN WITHOUT -RS TO ENSURE PROPER SEQUENCE
 */

#include "syscall.h"

int goodLockID, badLockID;

void LockThreadTest()
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
    Printf("\nTESTING LOCK ID FAILURE CONDITIONS\n\n", 37, 0, 0);
    /* Create with bad name string; ID should always be -1 */
    badLockID = CreateLock((char*)0, -1);
    /* Test syscalls with bad lock IDs */
    Acquire(badLockID);
    Release(badLockID);
    DestroyLock(badLockID);
    Acquire(1);
    Release(1);
    DestroyLock(1);
    /* Create with good name string; ID should always be 0 */
    goodLockID = CreateLock("Test1", 5);
    /* Test syscalls with good lock ID */
    Acquire(goodLockID);
    Release(goodLockID);
    DestroyLock(goodLockID);
    /* Test all syscalls with the now-deleted good lock */
    Acquire(goodLockID);
    Release(goodLockID);
    DestroyLock(goodLockID);
    /* PART 2: TEST MULTITHREADED LOCKS */
    Printf("\nTESTING LOCKS ON MULTIPLE THREADS\n\n", 36, 0, 0);
    /* Create a new lock, since the old one was deleted; ID should always be 1 */
    goodLockID = CreateLock("Test2", 5);
    /* Fork a new helper thread, then let it run immediately */
    Fork(LockThreadTest, "thread1", 7);
    Yield();
    /* Try to acquire the lock; won't finish until thread1 runs its "critical section" and does release/destroy/exit */
    Acquire(goodLockID);
    /* Run "critical section", then release and destroy for good */
    Printf("main critical section\n", 22, 0, 0);
    Release(goodLockID);
    DestroyLock(goodLockID);
    /* PART 3: TEST CROSS-PROCESS LOCK ACCESS */
    Printf("\nTESTING LOCKS ON MULTIPLE PROCESSES\n\n", 38, 0, 0);
    /* Create a new lock, since the old one was deleted; ID should always be 2 */
    goodLockID = CreateLock("Test3", 5);
    /* Exec a new helper process, then let it run immediately */
    Exec("../test/lockTestHelper", 22);
    Yield();
    /* Don't destroy the lock here; Exit() should take care of it */
}
