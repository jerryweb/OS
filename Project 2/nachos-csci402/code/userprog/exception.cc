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
	printf("%s","Bad pointer passed to Create\n");
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
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
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
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
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
	    printf("%s","Bad OpenFileId passed to Write\n");
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
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
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
      printf("%s","Tried to close an unopen file\n");
    }
}

  //*********************//
 // New syscalls below. //
//*********************//

void kernel_function(int vaddr)
// Sets up registers and stack space for a new thread.
{
    unsigned int addr = (unsigned int) vaddr;
    // Set program counter to new program.
    machine->WriteRegister(PCReg, addr);
    machine->WriteRegister(NextPCReg, addr + 4);\
    currentThread->space->RestoreState();
    // Allocate stack pages? (pretty sure this isn't how to do it)

    //currentThread->space->getNumPages() += 8;
    machine->WriteRegister(StackReg, currentThread->space->getNumPages() * PageSize - 16);
    // Run the new program.
    machine->Run();
}
void Fork_Syscall(unsigned int vaddr1, unsigned int vaddr2, int len)
// Creates and runs a new thread:
//  First, creates an address space for the thread from the parent process.
//  Next, give this thread its own set of stack pages.
//  Finally, fork new thread with an internal Nachos kernel fork.
// If there is an error, will return without forking the new thread.
{
    char *buf = new char[len+1];	// Kernel buffer: filename

    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in Exec\n");
        return;
    }

    if( copyin(vaddr2, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to Exec\n");
        delete[] buf;
        return;
    }

    buf[len]='\0';
    
    Thread* t = new Thread(buf); // Create new thread.
    t->space = currentThread->space; // Set the process to the currently running one.

    // update thread table
Thread->space->threadTable->Put(t);

    t->Fork(kernel_function, (int) vaddr1); // Fork the new thread to run the kernel program.
}

int Exec_Syscall(unsigned int vaddr, int len)
// (EXEC)
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
    
    // create new address space
    

    //add it to the process table
    // processTable->put(space);
    return 0;
}

void Yield_Syscall()
// Yields the current thread.
{
    currentThread->Yield();
    printf("Yield_Syscall was called; yielding current thread.\n");
}

void Exit_Syscall(int status)
// (EXIT)
{
    // currentThread->Finish();

}

void Acquire_Syscall(int id)
// Acquire the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without acquiring.
{

    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    if (currentThread->space != kLock->owner)
    {   // Check if current process has access to lock.
        // error
    }
    else
    {
        if (kLock->lock == NULL)
        {   // Make sure lock is valid. Should never reach here.
            // error
        }
        else
        {
            kLock->lock->Acquire();
            awakeThreadCount--;         //Decrements the number of 
                                        // threads that are active
        }
    }

}

void Release_Syscall(int id)
// Release the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without releasing.
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
            awakeThreadCount++;                     //increment the number of active threads
        }
    }
}

void Wait_Syscall(int id, int lockID)
// Waits on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without waiting.
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
            awakeThreadCount++;                     //increment the number of active threads

        }
    }
}
void Signal_Syscall(int id, int lockID)
// Signals the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without signalling.
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
            awakeThreadCount++;                     //increment the number of active threads

        }
    }
}
void Broadcast_Syscall(int id, int lockID)
// Broadcasts on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without broadcasting.
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
            //need to add the incrementer for the number of active threads
        }
    }
}

int CreateLock_Syscall(unsigned int vaddr, int len)
// Create a kernel lock with the name in the user buffer pointed
//  to by vaddr, with length len. If the lock is created successfully,
//  it is placed in the kernel lock table and its index is returned.
//  If there are any errors, -1 is returned.
{   char *buf = new char[len+1];	// Kernel buffer: name

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
// Create a kernel condition with the name in the user buffer pointed
//  to by vaddr, with length len. If the condition is created successfully,
//  it is placed in the kernel condition table and its index is returned.
//  If there are any errors, -1 is returned.
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
    
    KernelCondition* kCond = new KernelCondition;
    kCond->condition = new Condition(buf);
    kCond->owner = currentThread->space;
    kCond->isToBeDeleted = false;
    
    delete[] buf;
    
    return CVTable->Put(kCond);
}

void DestroyLock_Syscall(int id)
// (DESTROY LOCK)
{
    // bool threadWaitingForLock = false;

    // This will set the flag for the request for the lock to be 
    // deleted to true
    
    KernelLock* kLock = (KernelLock*) lockTable->Get(id);
    
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
        printf("%s","Invalid number of parameters in Printf\n");
    }
    
    char *buf = new char[len+1];	// Kernel buffer: name
    
    int parameters[4] = {0};
    int parameter = params;
    
    if (! buf)
    {
        printf("%s","Can't allocate kernel buffer in Printf\n");
        return;
    }

    if( copyin(vaddr, len, buf) == -1 )
    {
        printf("%s","Bad pointer passed to Printf\n");
        delete[] buf;
        return;
    }

    buf[len]='\0';
    
    for (int i = 0; i < numParams; i++)
    {
        parameters[i] = parameter % 1000;
        parameter / 1000;
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
            printf(buf, parameters[0], parameters[1]);
            break;
        case 3:
            printf(buf, parameters[0], parameters[1], parameters[2]);
            break;
        case 4:
            printf(buf, parameters[0], parameters[1], parameters[2], parameters[3]);
            break;
        default:
            printf("%s","Invalid number of parameters in Printf\n");
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
