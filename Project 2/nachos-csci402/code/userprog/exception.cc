// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "../threads/system.h"
#include "syscall.h"
#include "../threads/synch.h"
#include "../threads/thread.h"
#include "table.h"
#include <stdio.h>
#include <iostream>

using namespace std;

//table for locks
Table* lockTable;
struct KernelLock
{
  Lock* lock;
  AddrSpace* owner;
  bool isToBeDeleted;
};

//table for condition variables
Table* CVTable;
struct KernelCondition
{
  Condition* condition;
  AddrSpace* owner;
  bool isToBeDeleted;
};

//table for processes
Table* processTable;

void DestroyLock_Syscall(int id);
void DestroyCondition_Syscall(int id);

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occurs.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;            // The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
      {
            result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
      } 
      
      buf[n++] = *paddr;
     
      if ( !result ) {
    //translation failed
    return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occurs.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("Thread %s: Bad pointer passed to Create\n", currentThread->getName());
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("Thread %s: Can't allocate kernel buffer in Open\n", currentThread->getName());
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("Thread %s: Bad pointer passed to Open\n", currentThread->getName());
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("Thread %s: Error allocating kernel buffer for write!\n", currentThread->getName());
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("Thread %s: Bad pointer passed to to write: data not written\n", currentThread->getName());
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
        printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("Thread %s: Bad OpenFileId passed to Write\n", currentThread->getName());
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("Thread %s: Error allocating kernel buffer in Read\n", currentThread->getName());
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf( buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("Thread %s: Bad pointer passed to Read: data not copied\n", currentThread->getName());
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("Thread %s: Bad pointer passed to Read: data not copied\n", currentThread->getName());
		}
	    }
	} else {
	    printf("Thread %s: Bad OpenFileId passed to Read\n", currentThread->getName());
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("Thread %s: Tried to close an unopen file\n", currentThread->getName());
    }
}

/* New syscalls below. */

void kernel_function(int vaddr)
// Sets up registers and stack space for a new thread.
{
    printf("Thread %s: Entering kernel function\n", currentThread->getName());
    unsigned int addr = (unsigned int) vaddr;
    // Set program counter to new program.
    machine->WriteRegister(PCReg, addr);
    machine->WriteRegister(NextPCReg, addr + 4);
    currentThread->space->RestoreState();


    // (currentThread->space->getNumPages() - (currentThread->space->threadTable->getMaxCount() - currentThread->getThreadTableLocation()) * 8) * PageSize);//(currentThread->space->getNumPages() - 8) * PageSize );

    printf("stack pointer for thread %s: %d\n", currentThread->getName(), (currentThread->space->getNumPages() - (currentThread->space->threadTable->getMaxCount() - currentThread->getThreadTableLocation()) * 8) * PageSize);

    machine->WriteRegister(StackReg, (currentThread->space->getNumPages()- 8)  * PageSize);

    printf("Thread %s: Running\n", currentThread->getName());
    
    // Run the new program.
    machine->Run();
}

void Fork_Syscall(unsigned int vaddr1, unsigned int vaddr2, int len)
// Creates and runs a new thread in the current process. If there
//  is an error, will return without forking the new thread.
{
    char *buf = new char[len+1];	// Kernel buffer: filename
    
    if (! buf)
    {
        printf("Thread %s: Can't allocate kernel buffer in Fork\n", currentThread->getName());
        return;
    }

    if( copyin(vaddr2, len, buf) == -1 )
    {
        printf("Thread %s: Bad pointer %d passed to Fork\n", currentThread->getName(), vaddr2);
        delete[] buf;
        return;
    }

    buf[len]='\0';
    Thread* t = new Thread(buf); // Create new thread.
    t->space = currentThread->space; // Set the process to the currently running one.
    
    printf("Thread %s: Forking thread %s\n", currentThread->getName(), t->getName());

    //reallocate the page table

    t->space->setNewPageTable();
    // update thread table
    t->setThreadTableLocation(t->space->threadTable->Put(t));
    printf("Thread %s belongs to process %d\n", t->getName(), t->space->getID());
    // printf("Thread table location: %d\n", t->getThreadTableLocation());

    printf("thread table size %d\n", t->space->threadTable->getCount());

    t->Fork(kernel_function, (int) vaddr1); // Fork the new thread to run the kernel program.
}

void exec_thread(int n)
{
    /* Initialize the register by using currentThread->space. */
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
}

int Exec_Syscall(unsigned int vaddr, int len)
// Creates a new process and its kernel thread, and returns
//  the id of the process in the global processTable. Returns
//  -1 if the file name given is invalid.
{
    char *buf = new char[len+1];	// Kernel buffer: filename

    if (! buf)
    {
        printf("Thread %s: Can't allocate kernel buffer in Exec\n", currentThread->getName());
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Thread %s: Bad pointer %d passed to Exec\n", currentThread->getName(), vaddr);
        delete[] buf;
        return -1;
    }

    buf[len] = '\0';
    
    OpenFile* fileHandle = fileSystem->Open(buf);

    if (fileHandle == NULL) {
        printf("Unable to open file %s\n", buf);
        return -1;
    }

    
    AddrSpace* p = new AddrSpace(fileHandle);
    Thread* t = new Thread("Kernel thread");

    t->space = p;

    t->Fork(exec_thread, 0);
    
    int id = p->getID();
    p->threadTable->lockAcquire();
    printf("thread count for process %d: %d\n", p->getID(), p->threadTable->getCount());
    p->threadTable->lockRelease();

    return id;
}

void Yield_Syscall()
// Yields the current thread.
{
    printf("Thread %s: Yielding current thread.\n", currentThread->getName());
    currentThread->Yield();
}

void Exit_Syscall(int status)
// Exits the current running thread, accounting for the
//  number of remaining threads and processes. Parameter
//  is ignored for now.
{
    processTable->lockAcquire();
    
    AddrSpace* AddSP = currentThread->space;
    
    AddSP->threadTable->lockAcquire();
    
    int threadStackLoc = AddSP->getNumPages() - ((AddSP->threadTable->getMaxCount() - currentThread->getThreadTableLocation()) * 8);

    if(processTable->getCount() != 1){
        //use the known vpn indicies to find the corresponding ppn to clear and set the valid bit false

        // Reclaim all memory associated with the Address AddSP if this is the last 
        // thread in the process
        if(AddSP->threadTable->getCount() == 1){
            lockTable->lockAcquire();
            KernelLock* KL;
            for(int i = 0; i < lockTable->getCount(); i++){
                 KL = (KernelLock*) lockTable->Get(i);
                if(AddSP == KL->owner){
                    printf("Thread %s: DestroyLock called by exit\n", currentThread->getName());
                    DestroyLock_Syscall(i);
                    lockTable->lockAcquire();
                }
            }
            lockTable->lockRelease();
            
            CVTable->lockAcquire();
            KernelCondition* KC;
            for(int i = 0; i < CVTable->getCount(); i++){
                 KC = (KernelCondition*) CVTable->Get(i);
                if(AddSP == KC->owner){
                    printf("Thread %s: DestroyCondition called by exit\n", currentThread->getName());
                    DestroyCondition_Syscall(i);
                    CVTable->lockAcquire();
                }
            }
            CVTable->lockRelease();

            for(unsigned int i = 0; i < AddSP->getNumPages(); i++){
                if(memMap->Test(i)){
                    printf("clearing page %d for process %d\n", i, AddSP->getID());
                    memMap->Clear(i);
                }
            }
            processTable->Remove(AddSP->getID()); 
            //delete KL;    
        }

        else{
            processTable->lockRelease();
            
            for(int i = threadStackLoc; i < (threadStackLoc + 8); i++){
                if(memMap->Test(i)){
                    printf("clearing page %d for thread  hhh %s\n", i, currentThread->getName());
                    // printf("pageTable page %d valid set to %s\n", i, currentThread->space->getPageTableValidBit(i));
                    memMap->Clear(i);
                }
            }

        }
        AddSP->threadTable->Remove(currentThread->getThreadTableLocation());
        printf("calling current thread finish for thread %s\n", currentThread->getName());
        currentThread->Finish();
    }

    else{
        if(AddSP->threadTable->getCount() == 1){
            AddSP->threadTable->Remove(currentThread->getThreadTableLocation());

            KernelLock* KL;
            for(int i = 0; i < lockTable->getCount(); i++){
                 KL = (KernelLock*) lockTable->Get(i);
                if(AddSP == KL->owner){
                    printf("DestroyLock called by exit\n");
                    DestroyLock_Syscall(i);
                    //lockTable->Remove(i);
                }
            }

            for(unsigned int i = 0; i < AddSP->getNumPages(); i++){
                if(memMap->Test(i))
                    memMap->Clear(i);
            }
            //stop Nachos
            processTable->Remove(AddSP->getID()); 
            printf("Thread %s: Finishing last process, ending Nachos\n", currentThread->getName());
            interrupt->Halt();            
        }

        else{
            processTable->lockRelease();
            
            for(int i = threadStackLoc; i < (threadStackLoc + 8); i++){
                if(memMap->Test(i)){
                    //printf("clearing page %d for thread %s\n", i, currentThread->getName());
                    memMap->Clear(i);
                }
            }
            AddSP->threadTable->Remove(currentThread->getThreadTableLocation());
            printf("Thread %s: Finishing\n", currentThread->getName());
            currentThread->Finish();
        }
    }

}

void Acquire_Syscall(int id)
// Acquire the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without acquiring.
{
    lockTable->lockAcquire();
    
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (kLock == NULL || kLock->owner == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Thread %s: Trying to acquire invalid KernelLock, ID %d\n", currentThread->getName(), id);
        lockTable->lockRelease();
        return;
    }
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        printf("Thread %s: Trying to acquire other process's Lock, ID %d\n", currentThread->getName(), id);
        lockTable->lockRelease();
        return;
    }
    if (kLock->lock == NULL)
    {   // Make sure lock is valid. Should never reach here.
        printf("Thread %s: Trying to acquire invalid Lock, ID %d\n", currentThread->getName(), id);
        lockTable->lockRelease();
        return;
    }
    
    printf("Thread %s: Acquiring Lock, ID %d\n", currentThread->getName(), id);
    
    kLock->lock->Acquire();
    
    printf("Thread %s: Acquired Lock, ID %d\n", currentThread->getName(), id);
    
    lockTable->lockRelease();
}

void Release_Syscall(int id)
// Release the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without releasing.
{
    lockTable->lockAcquire();
    
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (kLock == NULL || kLock->owner == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Thread %s: Trying to release invalid KernelLock, ID %d\n", currentThread->getName(), id);
        lockTable->lockRelease();
        return;
    }
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        printf("Thread %s: Trying to release other process's Lock, ID %d\n", currentThread->getName(), id);
        lockTable->lockRelease();
        return;
    }
    if (kLock->lock == NULL)
    {   // Make sure lock is valid. Should never reach here.
        printf("Thread %s: Trying to release invalid Lock, ID %d\n", currentThread->getName(), id);
        lockTable->lockRelease();
        return;
    }
    
    printf("Thread %s: Releasing Lock, ID %d\n", currentThread->getName(), id);
    
    kLock->lock->Release();
    
    lockTable->lockRelease();
}

void Wait_Syscall(int id, int lockID)
// Waits on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without waiting.
{
    CVTable->lockAcquire();
    
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    if (kCond == NULL || kCond->owner == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Thread %s: Trying to wait on invalid KernelCondition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (kLock == NULL || kLock->owner == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Thread %s: Trying to wait using invalid KernelLock, ID %d\n", currentThread->getName(), lockID);
        CVTable->lockRelease();
        return;
    }
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        printf("Thread %s: Trying to wait on other process's Condition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    if (kCond->condition == NULL)
    {   // Make sure condition is valid. Should never reach here.
        printf("Thread %s: Trying to wait on invalid Condition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    
    printf("Thread %s: Waiting on Condition, ID %d\n", currentThread->getName(), id);
    
    kCond->condition->Wait(kLock->lock);
    
    printf("Thread %s: Waited on Condition, ID %d\n", currentThread->getName(), id);
    
    CVTable->lockRelease();
}
void Signal_Syscall(int id, int lockID)
// Signals the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without signalling.
{
    CVTable->lockAcquire();
    
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    if (kCond == NULL || kCond->owner == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Thread %s: Trying to signal invalid KernelCondition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (kLock == NULL || kLock->owner == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Thread %s: Trying to signal using invalid KernelLock, ID %d\n", currentThread->getName(), lockID);
        CVTable->lockRelease();
        return;
    }
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        printf("Thread %s: Trying to signal other process's Condition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    if (kCond->condition == NULL)
    {   // Make sure condition is valid. Should never reach here.
        printf("Thread %s: Trying to signal invalid Condition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    
    printf("Thread %s: Signalling Condition, ID %d\n", currentThread->getName(), id);
    
    kCond->condition->Signal(kLock->lock);
    
    CVTable->lockRelease();
}
void Broadcast_Syscall(int id, int lockID)
// Broadcasts on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without broadcasting.
{
    CVTable->lockAcquire();
    
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    if (kCond == NULL || kCond->owner == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Thread %s: Trying to broadcast on invalid KernelCondition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (kLock == NULL || kLock->owner == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Thread %s: Trying to broadcast using invalid KernelLock, ID %d\n", currentThread->getName(), lockID);
        CVTable->lockRelease();
        return;
    }
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        printf("Thread %s: Trying to broadcast on other process's Condition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    if (kCond->condition == NULL)
    {   // Make sure condition is valid. Should never reach here.
        printf("Thread %s: Trying to broadcast on invalid Condition, ID %d\n", currentThread->getName(), id);
        CVTable->lockRelease();
        return;
    }
    
    printf("Thread %s: Broadcasting on Condition, ID %d\n", currentThread->getName(), id);
    
    kCond->condition->Broadcast(kLock->lock);
    
    CVTable->lockRelease();
}

int CreateLock_Syscall(unsigned int vaddr, int len)
// Create a kernel lock with the name in the user buffer pointed
//  to by vaddr, with length len. If the lock is created successfully,
//  it is placed in the kernel lock table and its index is returned.
//  If there are any errors, -1 is returned.
{
    lockTable->lockAcquire();
    
    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("Thread %s: Can't allocate kernel buffer in CreateLock, ID -1\n", currentThread->getName());
        lockTable->lockRelease();
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Thread %s: Bad pointer %d passed to CreateLock, ID -1\n", currentThread->getName(), vaddr);
        delete[] buf;
        lockTable->lockRelease();
        return -1;
    }

    buf[len]='\0';
    
    KernelLock* kLock = new KernelLock;
    kLock->lock = new Lock(buf);
    kLock->owner = currentThread->space;
    kLock->isToBeDeleted = false;
    
    delete[] buf;
    
    int id = lockTable->Put(kLock);
    
    printf("Thread %s: Successfully created Lock, ID %d\n", currentThread->getName(), id);
    
    return id;
}

int CreateCondition_Syscall(unsigned int vaddr, int len)
// Create a kernel condition with the name in the user buffer pointed
//  to by vaddr, with length len. If the condition is created successfully,
//  it is placed in the kernel condition table and its index is returned.
//  If there are any errors, -1 is returned.
{
    CVTable->lockAcquire();
    
    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("Thread %s: Can't allocate kernel buffer in CreateCondition, ID -1\n", currentThread->getName());
        CVTable->lockRelease();
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Thread %s: Bad pointer %d passed to CreateCondition, ID -1\n", currentThread->getName(), vaddr);
        delete[] buf;
        CVTable->lockRelease();
        return -1;
    }

    buf[len]='\0';
    
    KernelCondition* kCond = new KernelCondition;
    kCond->condition = new Condition(buf);
    kCond->owner = currentThread->space;
    kCond->isToBeDeleted = false;
    
    delete[] buf;
    
    int id = CVTable->Put(kCond);
    
    printf("Thread %s: Successfully created Condition, ID %d\n", currentThread->getName(), id);
    
    return id;
}

void DestroyLock_Syscall(int id)
// (DESTROY LOCK)
{
    lockTable->lockAcquire();
    
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    
    if (kLock == NULL || kLock->owner == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Thread %s: Trying to destroy invalid KernelLock, ID %d\n", currentThread->getName(), id);
        return;
    }
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        printf("Thread %s: Trying to destroy other process's Lock, ID %d\n", currentThread->getName(), id);
        return;
    }
    
    if(!kLock->isToBeDeleted){
        kLock->isToBeDeleted = true;
        printf("Thread %s: Requesting to destroy Lock, ID %d\n", currentThread->getName(), id);
    }

    if((kLock->lock->getWaitQueue()->IsEmpty() &&
        kLock->lock->getOwner() == NULL &&
        kLock->isToBeDeleted)){

        printf("Thread %s: Destroying Lock, ID %d\n", currentThread->getName(), id);
        kLock->lock = NULL;
        kLock->owner = NULL;
    }
    else lockTable->lockRelease();
}
void DestroyCondition_Syscall(int id)
// (DESTROY CONDITION)
{
    CVTable->lockAcquire();
    
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    
    if (kCond == NULL || kCond->owner == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Thread %s: Trying to destroy invalid KernelCondition, ID %d\n", currentThread->getName(), id);
        return;
    }
    if (currentThread->space != kCond->owner)
    {   // Check if current process has access to condition.
        printf("Thread %s: Trying to destroy other process's Condition, ID %d\n", currentThread->getName(), id);
        return;
    }
    
    if(!kCond->isToBeDeleted){
        kCond->isToBeDeleted = true;
        printf("Thread %s: Requesting to destroy Condition, ID %d\n", currentThread->getName(), id);
    }

    if((kCond->condition->getWaitList()->IsEmpty() 
        && (kCond->condition->getWaitLock() == NULL)
        && kCond->isToBeDeleted)){
            
        printf("Thread %s: Destroying Condition, ID %d\n", currentThread->getName(), id);
        kCond->condition = NULL;
        kCond->owner = NULL;
    }
    else CVTable->lockRelease();
}

void Printf_Syscall(unsigned int vaddr, int len, int numParams, int params)
// Output the string in the user buffer pointed to by vaddr, with length len,
//  using printf(). Can take 0-4 parameters, separated by a factor of 100
//  in params.
{
    if (numParams < 0 || numParams > 4)
    {
        printf("Thread %s: Invalid number of parameters in Printf\n", currentThread->getName());
        return;
    }
    
    char *buf = new char[len+1];	// Kernel buffer: name
    
    int parameters[4] = {0};
    int parameter = params;
    
    if (! buf)
    {
        printf("Thread %s: Can't allocate kernel buffer in Printf\n", currentThread->getName());
        return;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Thread %s: Bad pointer passed to Printf\n", currentThread->getName());
        delete[] buf;
        return;
    }

    buf[len]='\0';
    
    for (int i = 0; i < numParams; i++)
    {
        parameters[i] = parameter % 100;
        parameter = parameter / 100;
    }
    switch (numParams)
    {
        case 0:
            printf(buf);
            break;
        case 1:
            printf(buf, parameters[0]);
            break;
        case 2:
            printf(buf, parameters[1], parameters[0]);
            break;
        case 3:
            printf(buf, parameters[2], parameters[1], parameters[0]);
            break;
        case 4:
            printf(buf, parameters[3], parameters[2], parameters[1], parameters[0]);
            break;
        default:
            printf("Thread %s: Invalid number of parameters in Printf\n", currentThread->getName());
            break;
    }
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
            DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
	    case SC_Create:
            DEBUG('a', "Create syscall.\n");
            Create_Syscall(machine->ReadRegister(4),
                           machine->ReadRegister(5));
            break;
	    case SC_Open:
            DEBUG('a', "Open syscall.\n");
            rv = Open_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5));
            break;
	    case SC_Write:
            DEBUG('a', "Write syscall.\n");
            Write_Syscall(machine->ReadRegister(4),
                          machine->ReadRegister(5),
                          machine->ReadRegister(6));
            break;
	    case SC_Read:
            DEBUG('a', "Read syscall.\n");
            rv = Read_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5),
                              machine->ReadRegister(6));
            break;
	    case SC_Close:
            DEBUG('a', "Close syscall.\n");
            Close_Syscall(machine->ReadRegister(4));
            break;
        // NEW SYSCALLS BELOW
	    case SC_Fork:
            DEBUG('a', "Fork syscall.\n");
            Fork_Syscall(machine->ReadRegister(4),
                         machine->ReadRegister(5),
                         machine->ReadRegister(6));
            break;
	    case SC_Exec:
            DEBUG('a', "Exec syscall.\n");
            rv = Exec_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5));
            break;
	    case SC_Yield:
            DEBUG('a', "Yield syscall.\n");
            Yield_Syscall();
            break;
	    case SC_Exit:
            DEBUG('a', "Exit syscall.\n");
            Exit_Syscall(machine->ReadRegister(4));
            break;
	    case SC_Acquire:
            DEBUG('a', "Acquire syscall.\n");
            Acquire_Syscall(machine->ReadRegister(4));
            break;
	    case SC_Release:
            DEBUG('a', "Release syscall.\n");
            Release_Syscall(machine->ReadRegister(4));
            break;
	    case SC_Wait:
            DEBUG('a', "Wait syscall.\n");
            Wait_Syscall(machine->ReadRegister(4),
                         machine->ReadRegister(5));
            break;
	    case SC_Signal:
            DEBUG('a', "Signal syscall.\n");
            Signal_Syscall(machine->ReadRegister(4),
                           machine->ReadRegister(5));
            break;
	    case SC_Broadcast:
            DEBUG('a', "Broadcast syscall.\n");
            Broadcast_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5));
            break;
	    case SC_CreateLock:
            DEBUG('a', "CreateLock syscall.\n");
            rv = CreateLock_Syscall(machine->ReadRegister(4),
                                    machine->ReadRegister(5));
            break;
	    case SC_CreateCondition:
            DEBUG('a', "CreateCondition syscall.\n");
            rv = CreateCondition_Syscall(machine->ReadRegister(4),
                                         machine->ReadRegister(5));
            break;
	    case SC_DestroyLock:
            DEBUG('a', "DestroyLock syscall.\n");
            DestroyLock_Syscall(machine->ReadRegister(4));
            break;
	    case SC_DestroyCondition:
            DEBUG('a', "DestroyCondition syscall.\n");
            DestroyCondition_Syscall(machine->ReadRegister(4));
            break;
	    case SC_Printf:
            DEBUG('a', "Printf syscall.\n");
            Printf_Syscall(machine->ReadRegister(4),
                           machine->ReadRegister(5),
                           machine->ReadRegister(6),
                           machine->ReadRegister(7));
            break;
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      printf("Thread %s: Unexpected user mode exception - which:%d  type:%d\n", currentThread->getName(), which, type);
      interrupt->Halt();
    }
}
