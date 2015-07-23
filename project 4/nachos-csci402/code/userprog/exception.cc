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
#include "../network/post.h"
#include <stdio.h>
#include <iostream>
#ifdef NETWORK
#include <sstream>
#include "serversynch.h"
#endif

using namespace std;

//state for server locks 
enum LockState {
	FREE, BUSY
};

//table for locks
Table* lockTable;
struct KernelLock {
	Lock* lock;
	AddrSpace* owner;
	bool isToBeDeleted;
};

//table for condition variables
Table* CVTable;
struct KernelCondition {
	Condition* condition;
	AddrSpace* owner;
	bool isToBeDeleted;
};

#ifdef NETWORK
Table* serverLockTable;
Table* serverCVTable;
Table* MVTable;
int createLockRequests,createCVRequests,createMVRequests;
#endif


//table for processes
Table* processTable;
Table* MVTable;
Table* LastTimeRecievedTable;
BitMap* memMap;

Lock* forkLock;
Lock* execLock;

void DestroyLock_Syscall(int id);
void DestroyCondition_Syscall(int id);
int GetMyBoxNumber_Syscall();

#ifdef NETWORK
//this integrated the receive message part of messaging
void serverResponseValidation() {

	PacketHeader inPktHdr;
	MailHeader inMailHdr;
	char buffer[MaxMailSize];

	// Wait for the first message from the other machine
	postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);

	printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from,
			inMailHdr.from);

	int responseValidation = 0;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << buffer;
	ss >> responseValidation;

	if (responseValidation == 1) {
		printf("Response from server was invalid. Terminating Nachos.\n");
		interrupt->Halt();
	}

	printf("Response from server was valid. Proceed.\n");

	//fflush(stdout);
}

int createLockResponse() {

	PacketHeader inPktHdr;
	MailHeader inMailHdr;
	char buffer[MaxMailSize];

	// Wait for the first message from the other machine
	postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);

	printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from,
			inMailHdr.from);

	int responseValidation = 0;
	int location;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << buffer;
	ss >> responseValidation;
	ss >> location;

	if (responseValidation == 1) {
		printf("Response from server was invalid. Terminating Nachos.\n");
		interrupt->Halt();
	}

	printf("Response from server was valid. Proceed.\n");
	return location;
}

//routine to send request to server
//difference from server msg function, the msg it not dynamiclly declared
void clientRequest(char* msg, int fromBox, int toBox) {
	PacketHeader outPktHdr;
	MailHeader outMailHdr;

	outPktHdr.to = 0;
	outPktHdr.from = 1;
	outMailHdr.to = toBox;
	outMailHdr.from = fromBox;
	outMailHdr.length = strlen(msg) + 1;

	postOffice->Send(outPktHdr, outMailHdr, msg);
}
#endif

int copyin(unsigned int vaddr, int len, char *buf) {
	// Copy len bytes from the current thread's virtual address vaddr.
	// Return the number of bytes so read, or -1 if an error occurs.
	// Errors can generally mean a bad virtual address was passed in.
	bool result;
	int n = 0;            // The number of bytes copied in
	int *paddr = new int;

	while (n >= 0 && n < len) {
		result = machine->ReadMem(vaddr, 1, paddr);
		while (!result) // FALL 09 CHANGES
		{
			result = machine->ReadMem(vaddr, 1, paddr); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
		}

		buf[n++] = *paddr;

		if (!result) {
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
	int n = 0;			// The number of bytes copied in

	while (n >= 0 && n < len) {
		// Note that we check every byte's address
		result = machine->WriteMem(vaddr, 1, (int) (buf[n++]));

		if (!result) {
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
	char *buf = new char[len + 1];	// Kernel buffer to put the name in

	if (!buf)
		return;

	if (copyin(vaddr, len, buf) == -1) {
		DEBUG('z', "Create: Thread %s: Bad pointer\n",
				currentThread->getName());
		delete buf;
		return;
	}

	buf[len] = '\0';

	fileSystem->Create(buf, 0);
	delete[] buf;
	return;
}

int Open_Syscall(unsigned int vaddr, int len) {
	// Open the file with the name in the user buffer pointed to by
	// vaddr.  The file name is at most MAXFILENAME chars long.  If
	// the file is opened successfully, it is put in the address
	// space's file table and an id returned that can find the file
	// later.  If there are any errors, -1 is returned.
	char *buf = new char[len + 1];	// Kernel buffer to put the name in
	OpenFile *f;			// The new open file
	int id;				// The openfile id

	if (!buf) {
		DEBUG('z', "Open: Thread %s: Can't allocate kernel buffer\n",
				currentThread->getName());
		return -1;
	}

	if (copyin(vaddr, len, buf) == -1) {
		DEBUG('z', "Open: Thread %s: Bad pointer\n", currentThread->getName());
		delete[] buf;
		return -1;
	}

	buf[len] = '\0';

	f = fileSystem->Open(buf);
	delete[] buf;

	if (f) {
		if ((id = currentThread->space->fileTable.Put(f)) == -1)
			delete f;
		return id;
	} else
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

	if (id == ConsoleInput)
		return;

	if (!(buf = new char[len])) {
		DEBUG('z', "Write: Thread %s: Error allocating kernel buffer\n",
				currentThread->getName());
		return;
	} else {
		if (copyin(vaddr, len, buf) == -1) {
			DEBUG('z', "Write: Thread %s: Bad pointer, data not written\n",
					currentThread->getName());
			delete[] buf;
			return;
		}
	}

	if (id == ConsoleOutput) {
		for (int ii = 0; ii < len; ii++) {
			DEBUG('z', "%c", buf[ii]);
		}

	} else {
		if ((f = (OpenFile *) currentThread->space->fileTable.Get(id))) {
			f->Write(buf, len);
		} else {
			DEBUG('z', "Write: Thread %s: Bad OpenFileId\n",
					currentThread->getName());
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

	if (id == ConsoleOutput)
		return -1;

	if (!(buf = new char[len])) {
		DEBUG('z', "Read: Thread %s: Error allocating kernel buffer\n",
				currentThread->getName());
		return -1;
	}

	if (id == ConsoleInput) {
		//Reading from the keyboard
		scanf(buf);

		if (copyout(vaddr, len, buf) == -1) {
			DEBUG('z', "Read: Thread %s: Bad pointer, data not copied\n",
					currentThread->getName());
		}
	} else {
		if ((f = (OpenFile *) currentThread->space->fileTable.Get(id))) {
			len = f->Read(buf, len);
			if (len > 0) {
				//Read something from the file. Put into user's address space
				if (copyout(vaddr, len, buf) == -1) {
					DEBUG('z',
							"Read: Thread %s: Bad pointer, data not copied\n",
							currentThread->getName());
				}
			}
		} else {
			DEBUG('z', "Read: Thread %s: Bad OpenFileId\n",
					currentThread->getName());
			len = -1;
		}
	}

	delete[] buf;
	return len;
}

void Close_Syscall(int fd) {
	// Close the file associated with id fd.  No error reporting.
	OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

	if (f) {
		delete f;
	} else {
		DEBUG('z', "Close: Thread %s: Tried to close an unopen file\n",
				currentThread->getName());
	}
}

/* New syscalls below. */

void kernel_function(int vaddr)
// Sets up registers and stack space for a new thread.
		{
	forkLock->Acquire();

	DEBUG('z', "Fork: Thread %s: Entering kernel function\n",
			currentThread->getName());
	unsigned int addr = (unsigned int) vaddr;
	// Set program counter to new program.
	machine->WriteRegister(PCReg, addr);
	machine->WriteRegister(NextPCReg, addr + 4);
	currentThread->space->RestoreState();

	int stackPage = currentThread->space->getNumPages()
			- (currentThread->space->threadTable->getMaxCount()
					- currentThread->getThreadTableLocation()) * 8;
	currentThread->setStackLocation(stackPage);
	DEBUG('z', "Fork: Thread %s: stack pointer %d; stack pages %d-%d\n",
			currentThread->getName(), (stackPage + 8) * PageSize, stackPage,
			stackPage + 7);

	machine->WriteRegister(StackReg, (stackPage + 8) * PageSize);

	DEBUG('z', "Fork: Thread %s: Running\n", currentThread->getName());

	forkLock->Release();
	// Run the new program.
	machine->Run();
}

void Fork_Syscall(unsigned int vaddr1, unsigned int vaddr2, int len)
// Creates and runs a new thread in the current process. If there
//  is an error, will return without forking the new thread.
		{
	forkLock->Acquire();

	if ((void*) vaddr1 == NULL) {
		DEBUG('z', "Fork: Thread %s: Invalid function pointer\n",
				currentThread->getName());
		return;
	}

	char *buf = new char[len + 1];	// Kernel buffer: filename

	if (!buf) {
		DEBUG('z', "Fork: Thread %s: Can't allocate kernel buffer\n",
				currentThread->getName());
		return;
	}

	if (copyin(vaddr2, len, buf) == -1) {
		DEBUG('z', "Fork: Thread %s: Bad pointer %d\n",
				currentThread->getName(), vaddr2);
		delete[] buf;
		return;
	}

	buf[len] = '\0';
	Thread* t = new Thread(buf); // Create new thread.
	t->space = currentThread->space; // Set the process to the currently running one.

	DEBUG('z', "Fork: Thread %s: Forking thread %s\n", currentThread->getName(),
			t->getName());

	//reallocate the page table

	t->space->setNewPageTable();
	// update thread table
	t->setThreadTableLocation(t->space->threadTable->Put(t));
	DEBUG('z', "Fork: Thread %s belongs to process %d\n", t->getName(),
			t->space->getID());

	DEBUG('z', "Fork: Thread table size %d\n",
			t->space->threadTable->getCount());

	forkLock->Release();

	t->Fork(kernel_function, (int) vaddr1); // Fork the new thread to run the kernel program.
}

void exec_thread(int n) {
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
	execLock->Acquire();

	char *buf = new char[len + 1];	// Kernel buffer: filename

	if (!buf) {
		DEBUG('z', "Exec: Thread %s: Can't allocate kernel buffer\n",
				currentThread->getName());
		return -1;
	}

	if (copyin(vaddr, len, buf) == -1) {
		DEBUG('z', "Exec: Thread %s: Bad pointer %d\n",
				currentThread->getName(), vaddr);
		delete[] buf;
		return -1;
	}

	buf[len] = '\0';

	OpenFile* fileHandle = fileSystem->Open(buf);

	if (fileHandle == NULL) {
		DEBUG('z', "Exec: Unable to open file %s\n", buf);
		return -1;
	}

	AddrSpace* p = new AddrSpace(fileHandle);
	Thread* t = new Thread("Kernel thread");

	t->space = p;

	t->Fork(exec_thread, 0);

	int id = p->getID();
	p->threadTable->lockAcquire();
	DEBUG('z', "Exec: Thread count for process %d: %d\n", p->getID(),
			p->threadTable->getCount());
	p->threadTable->lockRelease();

	execLock->Release();

	return id;
}

void Yield_Syscall()
// Yields the current thread.
{
	DEBUG('z', "Yield: Thread %s: Yielding current thread.\n",
			currentThread->getName());
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

	int threadStackLoc = AddSP->getNumPages()
			- ((AddSP->threadTable->getMaxCount()
					- currentThread->getThreadTableLocation()) * 8);
	printf("%s   The contents of register 4 is %d\n", currentThread->getName(),
			machine->ReadRegister(4));

	if (processTable->getCount() != 1) {
		//use the known vpn indicies to find the corresponding ppn to clear and set the valid bit false
		// Reclaim all memory associated with the Address AddSP if this is the last
		// thread in the process
		if (AddSP->threadTable->getCount() == 1) {
			lockTable->lockAcquire();
			KernelLock* KL;
			for (int i = 0; i < lockTable->getCount(); i++) {
				KL = (KernelLock*) lockTable->Get(i);
				if (AddSP == KL->owner) {
					DEBUG('z', "Exit: Thread %s: Calling DestroyLock\n",
							currentThread->getName());
					DestroyLock_Syscall(i);
					lockTable->lockAcquire();
				}
			}
			lockTable->lockRelease();

			CVTable->lockAcquire();
			KernelCondition* KC;
			for (int i = 0; i < CVTable->getCount(); i++) {
				KC = (KernelCondition*) CVTable->Get(i);
				if (AddSP == KC->owner) {
					DEBUG('z', "Exit: Thread %s: Calling DestroyCondition\n",
							currentThread->getName());
					DestroyCondition_Syscall(i);
					CVTable->lockAcquire();
				}
			}
			CVTable->lockRelease();

			for (unsigned int i = 0; i < AddSP->getNumPages(); i++) {
				TranslationEntryExec t = AddSP->getPageTable()[i];
				if (t.valid) {
					int ppn = t.physicalPage;
					ipt[ppn].valid = false;
					DEBUG('z', "Exit: setting physical page %d invalid\n", ppn);
				}
			}
			processTable->Remove(AddSP->getID());
		}

		else {
			processTable->lockRelease();

			for (int i = threadStackLoc; i < (threadStackLoc + 8); i++) {
				TranslationEntryExec t = AddSP->getPageTable()[i];
				if (t.valid) {
					int ppn = t.physicalPage;
					ipt[ppn].valid = false;
					DEBUG('z', "Exit: setting physical page %d invalid\n", ppn);
				}
			}

		}
		AddSP->threadTable->Remove(currentThread->getThreadTableLocation());
		DEBUG('z', "Exit: Thread %s: Finishing\n", currentThread->getName());
		currentThread->Finish();
	}

	else {
		if (AddSP->threadTable->getCount() == 1) {
			AddSP->threadTable->Remove(currentThread->getThreadTableLocation());

			KernelLock* KL;
			for (int i = 0; i < lockTable->getCount(); i++) {
				KL = (KernelLock*) lockTable->Get(i);
				if (AddSP == KL->owner) {
					DEBUG('z', "Exit: Thread %s: Calling DestroyLock\n",
							currentThread->getName());
					DestroyLock_Syscall(i);
				}
			}

			for (unsigned int i = 0; i < AddSP->getNumPages(); i++) {
				TranslationEntryExec t = AddSP->getPageTable()[i];
				if (t.valid) {
					int ppn = t.physicalPage;
					ipt[ppn].valid = false;
					DEBUG('z', "Exit: setting physical page %d invalid\n", ppn);
				}
			}
			//stop Nachos
			processTable->Remove(AddSP->getID());
			DEBUG('z',
					"Exit: Thread %s: Finishing last process, ending Nachos\n",
					currentThread->getName());
			interrupt->Halt();
		}

		else {
			processTable->lockRelease();

			for (int i = threadStackLoc; i < (threadStackLoc + 8); i++) {
				TranslationEntryExec t = AddSP->getPageTable()[i];
				if (t.valid) {
					int ppn = t.physicalPage;
					ipt[ppn].valid = false;
					DEBUG('z', "Exit: setting physical page %d invalid\n", ppn);
				}
			}
			AddSP->threadTable->Remove(currentThread->getThreadTableLocation());
			DEBUG('z', "Exit: Thread %s: Finishing\n",
					currentThread->getName());
			currentThread->Finish();
		}
	}

}

//void Acquire_Syscall(int id)
void Acquire_Syscall(int lock)
// Acquire the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without acquiring.
		{

#ifdef NETWORK
	//lockTable->lockAcquire();
	/*serverLock* sLock = (serverLock*) serverLockTable->Get(lock);
	if (sLock == NULL|| sLock->ownerID != -1)//currentThread->getThreadID())
	{   // Check if lock has been created (or not yet destroyed).
		DEBUG('n', "Thread %s: Trying to acquire invalid ServerLock, lock %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	if(sLock->name == NULL) {
		DEBUG('n', "Thread %s: Trying to acquire invalid Lock, ID %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}*/

	char* request;
	string toSend;
	stringstream ss;
	ss << "3 " << lock << " ";
	toSend = ss.str();
	request = (char*)toSend.c_str();

	printf("Acquire_Syscall:sending %s to server\n",request);

	clientRequest(request,0,0);
	serverResponseValidation();
#endif // NETWORK
}

void Release_Syscall(int lock)
// Release the kernel lock with the given ID. If the current process
//  does not have access to the lock or the lock does not exist,
//  will print an error without releasing.
		{
#ifdef NETWORK    
	// lockTable->lockAcquire();

	/*serverLock* sLock = (serverLock*) serverLockTable->Get(lock);
	//if (sLock == NULL|| sLock->ownerID != netAddr)
	if (sLock == NULL)
	{   // Check if lock has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to acquire invalid ServerLock, lock %d\n", currentThread->getName(), lock);
		// lockTable->lockRelease();
		return;
	}

	if(sLock->name == NULL) {
		DEBUG('z', "Thread %s: Trying to acquire invalid Lock, ID %d\n", currentThread->getName(), lock);
		// lockTable->lockRelease();
		return;
	}*/

	char* request;
	string toSend;
	stringstream ss;
	ss << "4 " << lock;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();

#endif // NETWORK
}

//void Wait_Syscall(int id, int lockID)
void Wait_Syscall(int lock, int CV)
// Waits on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without waiting.
		{
#ifdef NETWORK

	serverCV* sCond = (serverCV*) serverCVTable->Get(CV);
	if (sCond == NULL || sCond->name == NULL)
	{   // Check if condition has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to wait on invalid Server Condition, ID %d\n", currentThread->getName(), CV);
		return;
	}

	serverLock* sLock = (serverLock*) serverLockTable->Get(lock);
	if (sLock == NULL|| sLock->ownerID != -1) //currentThread->getThreadID())
	{   // Check if lock has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to acquire invalid ServerLock, lock %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	if(sLock->name == NULL) {
		DEBUG('z', "Thread %s: Trying to acquire invalid Lock, ID %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	char* request;
	string toSend;
	stringstream ss;
	ss << "8 " << sCond->name << " " <<sLock->name;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();

#endif // NETWORK
}

void Signal_Syscall(int lock, int CV) {
// Signals the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
//  an error without signalling.

#ifdef NETWORK

	serverCV* sCond = (serverCV*) serverCVTable->Get(CV);
	if (sCond == NULL || sCond->name == NULL)
	{   // Check if condition has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to wait on invalid Server Condition, ID %d\n", currentThread->getName(), CV);
		return;
	}

	serverLock* sLock = (serverLock*) serverLockTable->Get(lock);
	if (sLock == NULL|| sLock->ownerID != -1) //currentThread->getThreadID())
	{   // Check if lock has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to acquire invalid ServerLock, lock %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	if(sLock->name == NULL) {
		DEBUG('z', "Thread %s: Trying to acquire invalid Lock, ID %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	char* request;
	string toSend;
	stringstream ss;
	ss << "7 " << sCond->name << " " << sLock->name;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();
#endif // NETWORK
}

void Broadcast_Syscall(int lock, int CV)
// Broadcasts on the kernel condition with the given ID, using the kernel
//  lock with the given ID. If the current process does not have access
//  to the condition or the lock or either does not exist, will print
		//  an error without broadcasting.
		{
#ifdef NETWORK

	serverCV* sCond = (serverCV*) serverCVTable->Get(CV);
	if (sCond == NULL || sCond->name == NULL)
	{   // Check if condition has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to wait on invalid Server Condition, ID %d\n", currentThread->getName(), CV);
		return;
	}

	serverLock* sLock = (serverLock*) serverLockTable->Get(lock);
	if (sLock == NULL|| sLock->ownerID != -1) //currentThread->getThreadID())
	{   // Check if lock has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to acquire invalid ServerLock, lock %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	if(sLock->name == NULL) {
		DEBUG('z', "Thread %s: Trying to acquire invalid Lock, ID %d\n", currentThread->getName(), lock);
		//lockTable->lockRelease();
		return;
	}

	char buffer[MaxMailSize];

	char* request;
	string toSend;
	stringstream ss;
	ss << "9 " << sCond->name << " "<<sLock->name;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();

#endif // NETWORK
}

int CreateLock_Syscall(unsigned int vaddr, int len)
// Create a kernel lock with the name in the user buffer pointed
//  to by vaddr, with length len. If the lock is created successfully,
//  it is placed in the kernel lock table and its index is returned.
//  If there are any errors, -1 is returned.
		{
#ifdef NETWORK

	DEBUG('n',"in CreateLock_Syscall\n");

	char *buf = new char[len+1];	// Kernel buffer: name

	if (! buf)
	{
		DEBUG('n', "Thread %s: Can't allocate kernel buffer in CreateLock, ID -1\n", currentThread->getName());
		return -1;
	}

	if( copyin(vaddr, len, buf) == -1 )
	{
		DEBUG('n', "Thread %s: Bad pointer %d passed to CreateLock, ID -1\n", currentThread->getName(), vaddr);
		delete[] buf;
		return -1;
	}

	buf[len]='\0';

	char* request;
	string toSend;
	stringstream ss;
	ss << "1 " << buf;
	toSend = ss.str();
	request = (char*)toSend.c_str();
	printf("sending request %s\n",request);

	clientRequest(request,0,0);
	int lockLocation;
	lockLocation = createLockResponse();
	DEBUG('n',"Successfully created server Lock index %d \n",lockLocation);

	delete[] buf;
	return lockLocation;

	// DEBUG('z', "Thread %s: Successfully created server Lock, ID %d\n", currentThread->getName(), id);
#endif // NETWORK
	return 0;
}

int CreateCondition_Syscall(unsigned int vaddr, int len)
// Create a kernel condition with the name in the user buffer pointed
//  to by vaddr, with length len. If the condition is created successfully,
//  it is placed in the kernel condition table and its index is returned.
//  If there are any errors, -1 is returned.
		{
#ifdef NETWORK

	char *buf = new char[len+1];	// Kernel buffer: name

	if (! buf)
	{
		DEBUG('z', "Thread %s: Can't allocate kernel buffer in CreateCondition, ID -1\n", currentThread->getName());
		return -1;
	}

	if( copyin(vaddr, len, buf) == -1 )
	{
		DEBUG('z', "Thread %s: Bad pointer %d passed to CreateCondition, ID -1\n", currentThread->getName(), vaddr);
		delete[] buf;
		return -1;
	}

	buf[len]='\0';

	char* request;
	string toSend;
	stringstream ss;
	ss << "5 " << buf;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();
	delete[] buf;

	// DEBUG('z', "Thread %s: Successfully created Condition, ID %d\n", currentThread->getName(), id);
#endif // NETWORK
	return 0;
}

void DestroyLock_Syscall(int id)
// Destroys the kernel lock with the given ID. If there are threads in the
//  wait queue or any thread owns the lock, sets a flag instead. If the
//  current process does not have access to the lock or the lock does not
//  exist, will print an error without destroying or setting the flag. 
		{
#ifdef NETWORK

	/*serverLock* sLock = (serverLock*) lockTable->Get(id);

	if (sLock == NULL || sLock->name == NULL) //|| sLock->machineID != currentThread->get)
	{   // Check if lock has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to destroy invalid KernelLock, ID %d\n", currentThread->getName(), id);
		return;
	}*/

	char* request;
	string toSend;
	stringstream ss;
	ss << "2 " << id;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();
#endif // NETWORK
}
void DestroyCondition_Syscall(int CV)
// Destroys the kernel condition with the given ID. If there are threads
//  waiting on the condition, sets a flag instead. If the current process
//  does not have access to the condition or the condition does not
//  exist, will print an error without destroying or setting the flag.
		{
#ifdef NETWORK
	serverCV* sCond = (serverCV*) serverCVTable->Get(CV);
	if (sCond == NULL || sCond->name == NULL)
	{   // Check if condition has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to wait on invalid Server Condition, ID %d\n", currentThread->getName(), CV);
		return;
	}

	char* request;
	string toSend;
	stringstream ss;
	ss << "6 " << sCond->name;
	toSend = ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();
#endif // NETWORK
}

void Printf_Syscall(unsigned int vaddr, int len, int numParams, int params)
// Output the string in the user buffer pointed to by vaddr, with length len,
//  using DEBUG('z', ). Can take 0-4 parameters, separated by a factor of 100
//  in params.
{
	if (numParams < 0 || numParams > 4) {
		DEBUG('z', "Thread %s: Invalid number of parameters in Printf\n",
				currentThread->getName());
		return;
	}

	char *buf = new char[len + 1];	// Kernel buffer: name

	int parameters[4] = { 0 };
	int parameter = params;

	if (!buf) {
		DEBUG('z', "Thread %s: Can't allocate kernel buffer in Printf\n",
				currentThread->getName());
		return;
	}

	if (copyin(vaddr, len, buf) == -1) {
		DEBUG('z', "Thread %s: Bad pointer passed to Printf\n",
				currentThread->getName());
		delete[] buf;
		return;
	}

	buf[len] = '\0';

	for (int i = 0; i < numParams; i++) {
		parameters[i] = parameter % 100;
		parameter = parameter / 100;
	}
	switch (numParams) {
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
		DEBUG('z', "Thread %s: Invalid number of parameters in Printf\n",
				currentThread->getName());
		break;
	}
}

int GetID_Syscall() {
	return currentThread->getThreadID();
}

void SetID_Syscall(int id) {
	currentThread->setThreadID(id);
}

int GetMyBoxNumber_Syscall() {
	return currentThread->getMailBoxNum();
}

void SetMailBoxNum_Syscall() {
	currentThread->SetMailBoxNum();
}

//THIS NEEDS TO BE CONFIRMED 
int CreateMonitorVariable_Syscall(unsigned int vaddr, int len){
	char *buf = new char[len+1];

	if (! buf)
	{
		DEBUG('z', "Thread %s: Can't allocate kernel buffer in CreateMonitorVariable, ID -1\n", currentThread->getName());
		return -1;
	}

	if( copyin(vaddr, len, buf) == -1 )
	{
		DEBUG('z', "Thread %s: Bad pointer %d passed to CreateMonitorVariable, ID -1\n", currentThread->getName(), vaddr);
		delete[] buf;
		return -1;
	}

	buf[len]='\0';

	char* request;
	string toSend;
	stringstream ss;
	ss << "10 " << buf;
	toSend = ss.str();
	request = (char*)toSend.c_str();
	printf("sending request %s\n",request);

	clientRequest(request,0,0);
	int MVLocation;
	//TODO: either make a seperate createMVRequests or modify this name to keep consistancy
	MVLocation = createLockResponse();

	delete[] buf;
	return MVLocation;
}

//THIS NEEDS TO BE CONFIRMED 
void DestroyMonitorVariable_Syscall(int id){
	MonitorVariable* monVar = (MonitorVariable*) MVTable->Get(id);

	if (monVar == NULL || monVar->name == NULL) //|| monVar->machineID != currentThread->get)
	{   // Check if MV has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to destroy invalid Monitor Variable, ID %d\n", currentThread->getName(), id);
		return;
	}

	char* request;
	string toSend;
	stringstream ss;
	ss << "11 " << monVar->name;
	toSend == ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();
}

int GetMonitorVariable_Syscall(int indexPosition, int pos){
	MonitorVariable* monVar = (MonitorVariable*) MVTable->Get(pos);
	int location;
	/*
	if (monVar == NULL || monVar->name == NULL) //|| monVar->machineID != currentThread->get)
	{   // Check if MV has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to destroy invalid Monitor Variable, ID %d\n", currentThread->getName(), id);
		return;
	}

	char* request;
	string toSend;
	stringstream ss;
	ss << "12 " << monVar->name;
	toSend == ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();
	*/
	return location;	//this should be whatever value the server sends back
}


//TODO: modify the string to send the correct information
void SetMonitorVariable_Syscall(int indexPosition, int pos, int value){
	/*MonitorVariable* monVar = (MonitorVariable*) MVTable->Get(id);

	if (monVar == NULL || monVar->name == NULL) //|| monVar->machineID != currentThread->get)
	{   // Check if MV has been created (or not yet destroyed).
		DEBUG('z', "Thread %s: Trying to destroy invalid Monitor Variable, ID %d\n", currentThread->getName(), id);
		return;
	}

	char* request;
	string toSend;
	stringstream ss;
	ss << "13 " << monVar->name;
	toSend == ss.str();
	request = (char*)toSend.c_str();

	clientRequest(request,0,0);
	serverResponseValidation();*/
}


void ExceptionHandler(ExceptionType which) {
	int type = machine->ReadRegister(2); // Which syscall?
	int rv = 0; 	// the return value from a syscall

	if (which == SyscallException) {
		switch (type) {
		default:
			DEBUG('a', "Unknown syscall - shutting down.\n");
		case SC_Halt:
			DEBUG('a', "Shutdown, initiated by user program.\n");
			interrupt->Halt();
			break;
		case SC_Create:
			DEBUG('a', "Create syscall.\n");
			Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Open:
			DEBUG('a', "Open syscall.\n");
			rv = Open_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5));
			break;
		case SC_Write:
			DEBUG('a', "Write syscall.\n");
			Write_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),
					machine->ReadRegister(6));
			break;
		case SC_Read:
			DEBUG('a', "Read syscall.\n");
			rv = Read_Syscall(machine->ReadRegister(4),
					machine->ReadRegister(5), machine->ReadRegister(6));
			break;
		case SC_Close:
			DEBUG('a', "Close syscall.\n");
			Close_Syscall(machine->ReadRegister(4));
			break;
			// NEW SYSCALLS BELOW
		case SC_Fork:
			DEBUG('a', "Fork syscall.\n");
			Fork_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),
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
			Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		case SC_Signal:
			DEBUG('a', "Signal syscall.\n");
			Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
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
			Printf_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),
					machine->ReadRegister(6), machine->ReadRegister(7));
			break;
		case SC_GetID:
			DEBUG('a', "GetID syscall.\n");
			rv = GetID_Syscall();
			break;
		case SC_SetID:
			DEBUG('a', "SetID syscall.\n");
			SetID_Syscall(machine->ReadRegister(4));
			break;
		case SC_GetMyBoxNumber:
			DEBUG('a', "GetMyBoxNumber syscall.\n");
			rv = GetMyBoxNumber_Syscall();
			break;
		case SC_SetMailBoxNum:
			DEBUG('a', "SetMailBoxNum syscall.\n");
			SetMailBoxNum_Syscall();
			break;	
		case SC_CreateMonitorVariable:
			DEBUG('a', "CreateMonitorVariable syscall.\n");
			rv = CreateMonitorVariable_Syscall(machine->ReadRegister(4), 
				machine->ReadRegister(5));
			break;
		case SC_DestroyMonitorVariable:
			DEBUG('a', "DestroyMonitorVariable syscall.\n");
			DestroyMonitorVariable_Syscall(machine->ReadRegister(4));
			break;
		case SC_SetMonitorVariable:
			DEBUG('a', "SetMonitorVariable syscall.\n");
			SetMonitorVariable_Syscall(machine->ReadRegister(4), 
				machine->ReadRegister(5), machine->ReadRegister(6));
			break;
		case SC_GetMonitorVariable:
			DEBUG('a', "GetMonitorVariable syscall.\n");
			rv = GetMonitorVariable_Syscall(machine->ReadRegister(4),
			 machine->ReadRegister(5));
			break;
		}
		// Put in the return value and increment the PC
		machine->WriteRegister(2, rv);
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
		return;
	} else if (which == PageFaultException) {
		DEBUG('a', "Page fault.\n");
		// printf("Page Fault exception called!\n");
		currentThread->space->PageFault();
	} else {
		printf(
				"Thread %s: Unexpected user mode exception - which:%d  type:%d\n",
				currentThread->getName(), which, type);
		interrupt->Halt();
	}
}
