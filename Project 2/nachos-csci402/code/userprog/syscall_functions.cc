//This file holds all the function definitions used in exception.cc for syscalls 
#include "syscall.h"

using namespace std;


struct kernelLock
{
	Lock* lock;
	AddrSpace* owner;
	bool isToBeDeleted;
};

struct KernelCondition
{
	Condition* condition;
	AddrSpace* owner;
	bool isToBeDeleted;
};

void Fork_Syscall(unsigned int vaddr, int len)
{
    char *buf = new char[len+1];	// Kernel buffer: func

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in Fork\n");
        return;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to Fork\n");
        delete[] buf;
        return;
    }

    buf[len]='\0';
}

int Exec_Syscall(unsigned int vaddr, int len)
{
    char *buf = new char[len+1];	// Kernel buffer: filename

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in Exec\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to Exec\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    return 0;
}

void Yield_Syscall()
{
    currentThread->Yield();
    printf("Yield_Syscall was called; yielding current thread.\n");
}

void Exit_Syscall(int status)
{
    // currentThread->Finish();

}

void Acquire_Syscall(int id)
{

    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        // error
    }
    else
    {
        if (kLock->lock == NULL)
        {   // Make sure lock is valid.
            // error
        }
        else
        {
            kLock->lock->Acquire();
        }
    }

}

void Release_Syscall(int id)
{
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        // error
    }
    else
    {
        if (kLock->lock == NULL)
        {   // Make sure lock is valid.
            // error
        }
        else
        {
            kLock->lock->Release();
        }
    }
}

void Wait_Syscall(int id, int lockID)
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        // error
    }
    else
    {
        if (kLock->lock == NULL || kCond->condition == NULL)
        {   // Make sure condition and lock are valid.
            // error
        }
        else
        {
            kCond->condition->Wait(kLock->lock);
        }
    }
}
void Signal_Syscall(int id, int lockID)
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        // error
    }
    else
    {
        if (kLock->lock == NULL || kCond->condition == NULL)
        {   // Make sure condition and lock are valid.
            // error
        }
        else
        {
            kCond->condition->Signal(kLock->lock);
        }
    }
}
void Broadcast_Syscall(int id, int lockID)
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        // error
    }
    else
    {
        if (kLock->lock == NULL || kCond->condition == NULL)
        {   // Make sure condition and lock are valid.
            // error
        }
        else
        {
            kCond->condition->Broadcast(kLock->lock);
        }
    }
}

int CreateLock_Syscall(unsigned int vaddr, int len)
{
    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in CreateLock\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to CreateLock\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    KernelLock* kLock = new KernelLock;
    kLock->lock = new Lock(buf);
    kLock->owner = currentThread->space;
    kLock->isToBeDeleted = false;
    
    delete[] buf;
    
    return lockTable->Put(kLock);
}

int CreateCondition_Syscall(unsigned int vaddr, int len)
{

    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in CreateCondition\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to CreateCondition\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    KernelLock* kCond = new KernelCondition;
    kCond->lock = new Condition(buf);
    kCond->owner = currentThread->space;
    kCond->isToBeDeleted = false;
    
    delete[] buf;
    
    return CVTable->Put(kCond);
}
void DestroyLock_Syscall(int id)
{
    
}
void DestroyCondition_Syscall(int id)
{
    
}

void Printf_Syscall(unsigned int vaddr, int len, int param1, int param2)
{char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in CreateCondition\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to CreateCondition\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    printf(buf, param1, param2, param3);
}
