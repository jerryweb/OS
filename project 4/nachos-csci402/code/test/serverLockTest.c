#include "syscall.h"

int goodLockID, badLockID,cvID;

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

int main(){

	/* PART 1: TEST ID FAILURE CONDITIONS */
    /*Printf("\nTESTING LOCK ID FAILURE CONDITIONS\n\n", 37, 0, 0);*/
    /* Create with bad name string; ID should always be -1 */
    /*badLockID = CreateLock((char*)0, -1);*/
    /* Test syscalls with bad lock IDs */
    /*Acquire(badLockID);*/
    /*Release(badLockID);*/
    /*Acquire(1); */
    /*Release(1);*/
    /*DestroyLock(1);*/
    /* Create with good name string; ID should always be 0 */

    goodLockID = CreateLock("Test1", 5);
    Printf("Lock Test 1 created\n",20,0,0);
    /* Test syscalls with good lock ID */
    Acquire(goodLockID);
    Printf("Lock Test 1 acquired\n",21,0,0);
    cvID = CreateCondition("TestCV1",7);
    Printf("CV Test 1 created\n",18,0,0);
    Release(goodLockID);
    Printf("Lock Test 1 released\n",21,0,0);
    DestroyLock(goodLockID);
    Printf("Lock Test 1 destroyed\n",22,0,0);
    /* Test all syscalls with the now-deleted good lock */
    Acquire(goodLockID);
    Release(goodLockID);
    DestroyLock(goodLockID);
    /* PART 2: TEST MULTITHREADED LOCKS */
    Printf("\nTESTING LOCKS ON MULTIPLE THREADS\n\n", 36, 0, 0);
    /* Create a new lock, since the old one was deleted; ID should always be 1 */
    goodLockID = CreateLock("Test2", 5);
    /* Fork a new helper thread, then let it run immediately */
   /* Fork(LockThreadTest, "thread1", 7);*/
   /* Yield();*/
}
