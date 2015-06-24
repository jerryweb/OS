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

int awakeThreadCount = 0;

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
	printf("Bad pointer passed to Create\n");
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
	printf("Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("Bad pointer passed to Open\n");
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
	printf("Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("Bad pointer passed to to write: data not written\n");
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
	    printf("Bad OpenFileId passed to Write\n");
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
	printf("Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf( buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("Bad OpenFileId passed to Read\n");
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
      printf("Tried to close an unopen file\n");
    }
}

/* New syscalls below. */

void kernel_function(int vaddr)
// Sets up registers and stack space for a new thread.
{
    printf("entering kernel function for thread %s\n",
        currentThread->getName());
    unsigned int addr = (unsigned int) vaddr;
    // Set program counter to new program.
    machine->WriteRegister(PCReg, addr);
    machine->WriteRegister(NextPCReg, addr + 4);
    currentThread->space->RestoreState();

    machine->WriteRegister(StackReg, currentThread->space->getNumPages() * PageSize - 8);

    printf("running thread %s\n", currentThread->getName());
    
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
        printf("Can't allocate kernel buffer in Fork\n");
        return;
    }

    if( copyin(vaddr2, len, buf) == -1 )
    {
        printf("Bad pointer %d passed to Fork\n", vaddr2);
        delete[] buf;
        return;
    }

    buf[len]='\0';
    Thread* t = new Thread(buf); // Create new thread.
    t->space = currentThread->space; // Set the process to the currently running one.
    printf("Forking thread %s\n",
        t->getName());

    //reallocate the page table

    t->space->setNewPageTable();
    // update thread table
    t->space->threadTable->Put(t);
    // printf("thread table size %d\n", t->space->threadTable->getCount());

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
        printf("Can't allocate kernel buffer in Exec\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Bad pointer %d passed to Exec\n", vaddr);
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    OpenFile* fileHandle = fileSystem->Open(buf);
    
    AddrSpace* p = new AddrSpace(fileHandle);
    Thread* t = new Thread("Kernel thread");
    t->space = p;
    p->threadTable->Put(t);
    
    int id = processTable->Put(p);
    
    t->Fork(exec_thread, 0);
    
    return id;
}

void Yield_Syscall()
// Yields the current thread.
{
    printf("Yield_Syscall was called; yielding current thread.\n");
    currentThread->Yield();
}

void Exit_Syscall(int status)
// Exits the current running thread, accounting for the
//  number of remaining threads and processes. Parameter
//  is ignored for now.
{

    // where does currentThread->Finish() go?
    AddrSpace* space = currentThread->space;
    // if (space->threadTable->getCount() == 1)
    // {
       

    //     if (processTable->getCount() == 1)
    //     {
            
    //         stop nachos
    //         interrupt->Halt(c);
            
    //         printf("ending\n");
            
    //      }
    //     else
    //     {
    //         // reclaim all memory
    //          printf("calling current thread finish\n");
    //         currentThread->Finish();
    //     }
    // }

    // else
    // {
        /*
        reclaim stack:
            clear memory in bitmap
            set pageTableEntry to valid
        */
        printf("Thread table count %d\n", space->threadTable->getCount());
        printf("calling current thread finish for thread %s\n",
         currentThread->getName());
        currentThread->Finish();
    // }
}

void Acquire_Syscall(int id)
// Acquire the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without acquiring.
{

    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (kLock == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Trying to acquire invalid KernelLock, ID %d\n", id);
        return;
    }
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        printf("Trying to acquire other process's Lock, ID %d\n", id);
        return;
    }
    if (kLock->lock == NULL)
    {   // Make sure lock is valid. Should never reach here.
        printf("Trying to acquire invalid Lock, ID %d\n", id);
        return;
    }
    
    printf("Acquiring Lock, ID %d\n", id);
    
    kLock->lock->Acquire();
    awakeThreadCount--;         //Decrements the number of 
                                // threads that are active
    
    printf("Acquired Lock, ID %d\n", id);
}

void Release_Syscall(int id)
// Release the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without releasing.
{
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (kLock == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Trying to release invalid KernelLock, ID %d\n", id);
        return;
    }
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        printf("Trying to release other process's Lock, ID %d\n", id);
        return;
    }
    if (kLock->lock == NULL)
    {   // Make sure lock is valid.
        printf("Trying to release invalid Lock, ID %d\n", id);
        return;
    }
    
    printf("Releasing Lock, ID %d\n", id);
    
    kLock->lock->Release();
    awakeThreadCount++;                     //increment the number of active threads
    
    printf("Released Lock, ID %d\n", id);
    
}

void Wait_Syscall(int id, int lockID)
// Waits on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without waiting.
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    if (kCond == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Trying to wait on invalid KernelCondition, ID %d\n", id);
        return;
    }
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        printf("Trying to wait on other process's Condition, ID %d\n", id);
        return;
    }
    if (kCond->condition == NULL)
    {   // Make sure condition is valid.
        printf("Trying to wait on invalid Condition, ID %d\n", id);
        return;
    }
    
    printf("Waiting on Condition, ID %d\n", id);
    
    kCond->condition->Wait(kLock->lock);
    awakeThreadCount++;                     //increment the number of active threads
    
    printf("Waited on Condition, ID %d\n", id);
    
}
void Signal_Syscall(int id, int lockID)
// Signals the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without signalling.
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    if (kCond == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Trying to signal invalid KernelCondition, ID %d\n", id);
        return;
    }
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        printf("Trying to signal other process's Condition, ID %d\n", id);
        return;
    }
    if (kCond->condition == NULL)
    {   // Make sure condition is valid.
        printf("Trying to signal invalid Condition, ID %d\n", id);
    return;
    }
    
    printf("Signalling Condition, ID %d\n", id);
    
    kCond->condition->Signal(kLock->lock);
    awakeThreadCount++;                     //increment the number of active threads
    
    printf("Signalled Condition, ID %d\n", id);
}
void Broadcast_Syscall(int id, int lockID)
// Broadcasts on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without broadcasting.
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    if (kCond == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Trying to broadcast on invalid KernelCondition, ID %d\n", id);
        return;
    }
    KernelLock* kLock = (KernelLock*) lockTable->Get(lockID);
    if (currentThread->space != kLock->owner || currentThread->space != kCond->owner)
    {   // Check if current process has access to condition and lock.
        printf("Trying to broadcast on other process's Condition, ID %d\n", id);
        return;
    }
    if (kCond->condition == NULL)
    {   // Make sure condition is valid.
        printf("Trying to broadcast on invalid Condition, ID %d\n", id);
        return;
    }
    
    printf("Broadcasting on Condition, ID %d\n", id);
    
    kCond->condition->Broadcast(kLock->lock);
    //need to add the incrementer for the number of active threads
    
    printf("Broadcasted on Condition, ID %d\n", id);
}

int CreateLock_Syscall(unsigned int vaddr, int len)
// Create a kernel lock with the name in the user buffer pointed
//  to by vaddr, with length len. If the lock is created successfully,
//  it is placed in the kernel lock table and its index is returned.
//  If there are any errors, -1 is returned.
{   char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("Can't allocate kernel buffer in CreateLock\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Bad pointer %d passed to CreateLock\n", vaddr);
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    KernelLock* kLock = new KernelLock;
    kLock->lock = new Lock(buf);
    kLock->owner = currentThread->space;
    kLock->isToBeDeleted = false;
    
    delete[] buf;
    
    int id = lockTable->Put(kLock);
    
    printf("Successfully created Lock, ID %d\n", id);
    
    return id;
}

int CreateCondition_Syscall(unsigned int vaddr, int len)
// Create a kernel condition with the name in the user buffer pointed
//  to by vaddr, with length len. If the condition is created successfully,
//  it is placed in the kernel condition table and its index is returned.
//  If there are any errors, -1 is returned.
{

    char *buf = new char[len+1];	// Kernel buffer: name

    if (! buf)
    {
        printf("Can't allocate kernel buffer in CreateCondition\n");
        return -1;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Bad pointer %d passed to CreateCondition\n", vaddr);
        delete[] buf;
        return -1;
    }

    buf[len]='\0';
    
    KernelCondition* kCond = new KernelCondition;
    kCond->condition = new Condition(buf);
    kCond->owner = currentThread->space;
    kCond->isToBeDeleted = false;
    
    delete[] buf;
    
    int id = CVTable->Put(kCond);
    
    printf("Successfully created Condition, ID %d\n", id);
    
    return id;
}

void DestroyLock_Syscall(int id)
// (DESTROY LOCK)
{
    // bool threadWaitingForLock = false;

    // This will set the flag for the request for the lock to be 
    // deleted to true
    
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    
    if (kLock == NULL)
    {   // Check if lock has been created (or not yet destroyed).
        printf("Trying to delete invalid KernelLock, ID %d\n", id);
        return;
    }
    
    if(!kLock->isToBeDeleted){
        kLock->isToBeDeleted = true;
        printf("Syscall request to destroy lock %d\n", id);
    }

    if((kLock->lock->getWaitQueue()->IsEmpty() &&
        kLock->lock->getReadyQueue()->IsEmpty() &&
        kLock->isToBeDeleted) || (awakeThreadCount == 0)){

        lockTable->lockAcquire();     // prevent lock corruption when 
                                        // deleting the lock
        printf("Syscall destroying lock %d\n", id);
        kLock->lock = NULL;
        kLock->owner = NULL;  

        lockTable->lockRelease();
    }
    
    
}
void DestroyCondition_Syscall(int id)
// (DESTROY CONDITION)
{
    KernelCondition* kCond = (KernelCondition*) CVTable->Get(id);
    
    if (kCond == NULL)
    {   // Check if condition has been created (or not yet destroyed).
        printf("Trying to delete invalid KernelCondition, ID %d\n", id);
        return;
    }
    
    if(!kCond->isToBeDeleted){
        kCond->isToBeDeleted = true;
        printf("Syscall request to destroy condition %d\n", id);
    }

    if((kCond->condition->getWaitList()->IsEmpty() 
        && (kCond->condition->getWaitLock() == NULL)
        && kCond->isToBeDeleted) || (awakeThreadCount == 0)){

        CVTable->lockAcquire();   // prevent lock corruption when 
                                    // deleting the condition
        printf("Syscall destroying condition %d\n", id);
        kCond->condition = NULL;
        kCond->owner = NULL;

        CVTable->lockRelease();
    }


}

void Printf_Syscall(unsigned int vaddr, int len, int numParams, int params)
// Output the string in the user buffer pointed to by vaddr, with length len,
//  using printf(). Can take 0-4 parameters, separated by a factor of 1000
//  in params.
{
    if (numParams < 0 || numParams > 4)
    {
        printf("Invalid number of parameters in Printf\n");
        return;
    }
    
    char *buf = new char[len+1];	// Kernel buffer: name
    
    int parameters[4] = {0};
    int parameter = params;
    
    if (! buf)
    {
        printf("Can't allocate kernel buffer in Printf\n");
        return;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("Bad pointer passed to Printf\n");
        delete[] buf;
        return;
    }

    buf[len]='\0';
    
    for (int i = 0; i < numParams; i++)
    {
        parameters[i] = parameter % 1000;
        parameter = parameter / 1000;
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
            printf("Invalid number of parameters in Printf\n");
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
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
