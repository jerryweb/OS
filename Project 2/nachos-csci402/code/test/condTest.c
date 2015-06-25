/* condTest.c
 *	Simple program to test whether Condition syscalls work.
 *  Runs through every possible case of every syscall.
 *  MUST BE RUN WITHOUT -RS TO ENSURE PROPER SEQUENCE
 */

int goodLockID1, goodLockID2, badLockID;
int goodCondID, badCondID;

void CondThreadTest1()
{
    /* Try syscalls with incorrect lock */
    Wait(goodCondID, goodLockID2);
    Signal(goodCondID, goodLockID2);
    Broadcast(goodCondID, goodLockID2);
    /* Try to destroy the condition first, then signal and wait - it shouldn't delete until the end of the section */
    Acquire(goodLockID1);
    DestroyCondition(goodCondID);
    Signal(goodCondID, goodLockID1);
    Wait(goodCondID, goodLockID1);
    Exit(0);
}
void CondThreadTest2()
{
    /* Try to destroy the condition first, then broadcast and wait - it shouldn't delete until the end of the section */
    DestroyCondition(goodCondID);
    Broadcast(goodCondID, goodLockID1);
    Exit(0);
}

int
main()
{
    badLockID = CreateLock((char*)0, -1);
    goodLockID1 = CreateLock("Lock1", 5);
    goodLockID2 = CreateLock("Lock2", 5);
    /* PART 1: TEST ID FAILURE CONDITIONS */
    Printf("\nTESTING CONDITION ID FAILURE CONDITIONS\n\n", 42, 0, 0);
    /* Create with bad name string: ID -1 */
    badCondID = CreateCondition((char*)0, -1);
    /* Create with good name string: ID 0 */
    goodCondID = CreateCondition("Test1", 5);
    /* Test syscalls with bad IDs */
    Wait(badCondID, goodLockID1);
    Signal(badCondID, goodLockID1);
    Broadcast(badCondID, goodLockID1);
    DestroyCondition(badCondID);
    Wait(1, goodLockID1);
    Signal(1, goodLockID1);
    Broadcast(1, goodLockID1);
    DestroyCondition(1);
    Wait(goodCondID, badLockID);
    Signal(goodCondID, badLockID);
    Broadcast(goodCondID, badLockID);
    /* PART 2: TEST MULTITHREADED CONDITIONS */
    Printf("\nTESTING CONDITIONS ON MULTIPLE THREADS\n\n", 41, 0, 0);
    /* Fork a new helper thread v1, then wait on the condition */
    Fork(CondThreadTest1, "thread1", 7);
    Acquire(goodLockID1);
    Wait(goodCondID, goodLockID1);
    /* Fork a new helper thread v2, then wait on the condition again */
    Fork(CondThreadTest2, "thread2", 7);
    Acquire(goodLockID1);
    Wait(goodCondID, goodLockID1);
    /* Destroy the condition once woken up */
    DestroyCondition(goodCondID);
    /* Test syscalls on now-deleted condition */
    Wait(goodCondID, goodLockID1);
    Signal(goodCondID, goodLockID1);
    Broadcast(goodCondID, goodLockID1);
    DestroyCondition(goodCondID, goodLockID1);
    /* PART 3: TEST CROSS-PROCESS CONDITION ACCESS */
    Printf("\nTESTING CONDITION ON MULTIPLE PROCESSES\n\n", 42, 0, 0);
    /* Create a new condition, since the old one was deleted; ID should always be 1 */
    goodCondID = CreateCondition("Test2", 5);
    /* Exec a new helper process, then let it run immediately */
    Exec("../test/condTestHelper", 22);
    Yield();
    /* Don't destroy the condition and locks here; Exit() should take care of it */
}
