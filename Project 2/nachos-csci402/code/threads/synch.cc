// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}
//----------------------------------------------------------------------
// Lock::Lock
//  Initialize a lock, so that it can be used for mutual exclusion of 
//  critical sections.
//
//  "debugName" is an arbitrary name, useful for debugging
//  waitQueue is the list of threads waiting to wake up (not available to the scheduler)
//  lockState indicates whether the lock is under a threads possesion or free to use
//  lockOwner is the thread that currently owns the lock (current thread)
//----------------------------------------------------------------------
Lock::Lock(char* debugName) {
    name = debugName;
    waitQueue = new List();
    lockState = FREE;
    lockOwner = NULL;
}

//----------------------------------------------------------------------
// Lock::~Lock
//  De-allocate Lock, when no longer needed.  Assume no one
//  is still waiting on the Lock
//----------------------------------------------------------------------
Lock::~Lock() {
    delete waitQueue;
}

//  These are the atomic operations used for aquiring and releasing the lock
//  

void Lock::Acquire() {
    //printf("%s acquires %s\n", currentThread->getName(), name);

    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    if(isHeldByCurrentThread()){ //currentThread is already the lockOwner of the lock
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    if(lockState == FREE){ //lock is available
        lockState = BUSY;         //set the state of the lock to busy
        lockOwner = currentThread;      //set the lockOwner of the lock to the current thread
    }

    else{ //lock not available
        waitQueue->Append((void *)currentThread); //add currentThread to waiting queue
        currentThread->Sleep();                   //put currentThread to sleep
    }


    (void) interrupt->SetLevel(oldLevel);  //reenable interrupts
}

void Lock::Release() {
    //printf("%s releases %s\n", currentThread->getName(), name);

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    if(!isHeldByCurrentThread()){           //prints an error message if currentThread
                                            //not the lockOwner
        printf("Lock::Release: This thread is not the current lockOwner of lock %s\n", name);
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    if(!waitQueue->IsEmpty()){
        Thread *thread;
        thread = (Thread *)waitQueue->Remove();
        
        if (thread != NULL){    // make thread ready, consuming the V immediately
            scheduler->ReadyToRun(thread);
            lockOwner = thread;
        }
    }

    else{
        lockState = FREE;    //set the state of he lock to free
        lockOwner = NULL;       // set lock thread pointer back to null
    }

    (void) interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread(){
    return lockOwner == currentThread;
}

Condition::Condition(char* debugName)
{
    name = debugName;
    waitLock = NULL;
    waitList = new List();
}
Condition::~Condition()
{
    delete waitList;
    // delete waitLock;
}
void Condition::Wait(Lock* conditionLock)
{
    //printf("%s waits on %s\n", currentThread->getName(), name);

    IntStatus old = interrupt->SetLevel(IntOff);
    
    if (conditionLock == NULL)
    {   // Make sure that the parameter is a valid lock.
        printf("Condition::Wait (%s): parameter conditionLock is not a valid Lock*\n", name);
        interrupt->SetLevel(old);
        return;
    }
    if (waitLock == NULL)
    {   // Check if another thread has called wait.
        waitLock = conditionLock;
    }
    else if (waitLock != conditionLock )
    {   // Make sure that the caller is trying to access the right lock.
        printf("Condition::Wait (%s): parameter conditionLock (%s) is not the same as the waitLock (%s)\n",
                name, conditionLock->getName(), waitLock->getName());
        interrupt->SetLevel(old);
        return;
    }
    conditionLock->Release();
    waitList->Append(currentThread);
    currentThread->Sleep();
    //conditionLock->Acquire();
    interrupt->SetLevel(old);
}

void Condition::Signal(Lock* conditionLock)
{
    //printf("%s signals %s\n", currentThread->getName(), name);

    IntStatus old = interrupt->SetLevel(IntOff);
    
    if (waitList->IsEmpty())
    {   // Nothing to do if no waiting threads.
        interrupt->SetLevel(old);
        return;
    }
    if (conditionLock == NULL)
    {   // Make sure that the parameter is a valid lock.
        printf("Condition::Signal (%s): parameter conditionLock is not a valid Lock*\n", name);
        interrupt->SetLevel(old);
        return;
    }
    if (waitLock != conditionLock)
    {   // Make sure that the caller is trying to access the right lock.
        printf("Condition::Wait (%s): parameter conditionLock (%s) is not the same as the waitLock (%s)\n",
                name, conditionLock->getName(), waitLock->getName());
        interrupt->SetLevel(old);
        return;
    }
    Thread* thread = (Thread*)waitList->Remove();
    scheduler->ReadyToRun(thread);
    if (waitList->IsEmpty()) { waitLock = NULL; }
    interrupt->SetLevel(old);
}
void Condition::Broadcast(Lock* conditionLock)
{
    //printf("%s broadcasts on %s\n", currentThread->getName(), name);

    if (conditionLock == NULL)
    {   // Make sure that the parameter is a valid lock.
        printf("Condition::Broadcast (%s): parameter conditionLock is not a valid Lock*\n", name);
        return;
    }
    if (waitLock != conditionLock)
    {   // Make sure that the caller is trying to access the right lock.
        printf("Condition::Broadcast (%s): parameter conditionLock (%s) is not the same as the waitLock (%s)\n",
                name, conditionLock->getName(), waitLock->getName());
        return;
    }
    else
    {
        while (! waitList->IsEmpty())
        {
            Signal(conditionLock);
        }
    }
}
