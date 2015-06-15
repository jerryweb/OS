System Calls
	-mechanism by which the O.S allows user programs to perform privileged tasks Nachos has such a mechanism
	-The implementation of syscall goes in exception.cc
		-exception handler //equivalent of the interrupt mechanism
							//This is where Nachos code currently exists
		-6 already there	//reuse the code already there for handling the passed in parameters


		Yield
			currentThread->Yield();	//add a print statment after this to allow for visible testing

		CreateLock
		DestroyLock
		Acquire 
		Release
		CreateCV
		DestroyCV
		Wait 
		Signal 
		Broadcast
		Fork
		Exec 
		Exit 


		Lock code 
		Condition code
		Forking a thread
			.
			.
			.
		syscalls are a bridge from user mode to kernel mode		//The user can't have direct access to the hardware
		Syscalls look like function calls, but are not			//But it isn't just a function because they are not very safe
			Public interface => Syscall.h						//Thing that looks like a function
									-syscall #
									-function look-a-like
					start.s  									//acts like an interrupt for hardware and goes to execption handler
						8-9 lines of code						//if data, Machos turns it into assembly code only up to 4 arguement
						#define SC_Yield 10

			When an interrupt occurs, eventuaally the interrupt handler will start up //all it knows there is an interrupt, but not what kind it is
				Task 1: Must determine the interrupt type 
						|
						V
					Raise exception
						-figures out syscall execption
								|
								V
							Exception handler 					//Don't write the implementation here or the file will be too long
								//start here at exception handler

	Pattern for Syscall implementation
		1. validate all data coming from the userprogram 		//Need to have a test to show that it will not except bad data(negative test)
			[You must have tests showing your validation works]
		2. if all data is valid, carry out the tasks
		3. (optional) Return a value to userprogram

	How to get user data
		-Nachos has 40 registers //0 through 39
		registers 4 to 7 have user data //this is why you can only have 4 arguements: 1st goes into 4, 2nd into 5, etc.
		mahcine->WriteRegister(	);
				 ReadRegister(	);
		
		//Nachos is both pure C and MIPS program 
		mipssim.cc 
			data types			//These are the only ones that work with MIPS
				-int 			//these are the only arguements that can be passed
				-char			//The actual char array data is not stored in the register, but the address pointer
					//Copyin translates this point in virtual memory to the actual data used in kernel mode
			no heap - no malloc

	CreateLock
		int CreateLock(char*, int);
		Return values are never pointers into the OS data structures
		It is a number that can be verified belonging to the Process trying to use it 


	doCreate.c 
		#include "syscall.h"
		int main(){
			int c = CreateLock("myLock",6);	//There isn't a NULL byte so you need to pass the # of characters are in the char array 
										/	//Nachos can't count the length on it's own
									   /
									Sizeof("myLock")	//Use this for long strings
			c = 0; // or c++ etc.
			Acquire(c);	//c should be the index number into the array of locks and conditinos
				- We return a value that cannot damage the OS if changed
				-Protect processes from each other
		}

ExceptionHandler
	case SC_CreateLock:
			.
			.
		Lock* l = new Lock(___);

		rv = (int)l;	//dont do this (need verification)

We need a table of:
	Locks - Lock*
	Condtion variables - Condtion* 
	lockTable			//Acquire(index of lockTable) 
	//The table should go into system.h and system.cc
	----------------------------------------------
	|___|Lock*|Process Owner (PID, addressSpace*)| //If you want to use PID, change the thread class
	|___|_____|__________________________________|	//The addressSpace* is currentThread->Space
   5|	|	  |									 | // owner = NULL;
	|	|     |									 |
	----------------------------------------------
	array of 
	int nextLockLocation = 0;
	//You are NOT required to reuse slots int the table once a lock is destroyed

	//check if this is owning process
	lockTable[value /*from user*/].lock->Acquire();	//lockTable[value].lock is a Lock*
	Lock*								->Release();

	//Until you have exec working, you only have one user program
	//Do Fork First 
	//Once you have Exec done, you can have multiple processes stored in a process table
	//2 airport simulations at least 

	int c; 	//shared variable
	Thread A 						Thread B  	//Thread A get more CPU time than A, resulting 
												//in the lock being destroyed before B can use it
	int main(){						int main(){
		c = CreateLock(__);
		Acquire(c);						Acquire(c);

		Release(c);						Release(c);		//lock state will be *AVAILABLE	after B is doen using it	
		*DestroyLock(C);	//only destroy if available			
	}								}


	stuct kernelLock{
		Lock* Lock;	//can either set it to NULL, which will be checked when the Aquire and Release calls are used
		AddrSpace* owner;	//Don't actaully delete this; can just set it to NULL
		bool isToBeDeleted;
		bool isDeleted ///optional if you don't set Lock* to NULL
	}
	//fist make sure the lock exists, then make sure the lock doing the release or acquire is the owner

	system.cc
		Lock* lockTableLock;


		void main(){
			Initialized();			//This is in system.cc and is where all of nachos data structures are initialized here 
									//Our new data structure should go in this function under everything else
				// _init		these two go in the 'function'
				// _start
			lockTableLock->Acquire();
		}

Hidden Requirement
	Part 3 - airport
		-output guidelines 
	Nachos - no printf(); statement
			//only ReadSyscall and WriteSyscall
			//WriteSyscall only takes a char*
			Pass 1 .... 2 .... 3

		//write a syscall to printf
		SC_Printf(char*, int, int, int){
			printf("_____ %d _____ %d_____\n",_,_);	
			
			(#1 * 100) + #2
			//#1 is the liaison # and #2 is the passenger #	
		}

Fork,Exec,Exit 
	Fork - adds a new thread to an existing process
		 - each thread MUST have its own stack

	Exec - creates a new single-threaded process //new addrSpace

	nachos -x testfiles
		// -x is where StartProcess /*new thread ready created*/ is called
		in progtest.cc

	Exit - must be called by every Forked thread only //if you don't, you may get an illegal instruction exception 
												 	  //because the PC may go into the data section of memory
		 - not needed in main
		 Need at least
		 	currentThread->Finish();
		 	//once you get Fork and Exec working you can start on the full exit cases
		 - Full Exit - 3 cases
		 	- Goal: Reclaim whatever resources no longer needed
		 		-memory
		 		-Locks 								//forget about using destroys in the airport simulation
		 		-CVs 
		 		//if you were to implement, the manager would do all of the destroys
		 	Case 1: Last executing thread in lastprocess
		 			stop Nachos
		 			interrupt->Halt(c);

		 	Case 2: A thread in a process, not the last thread
		 			Reclaim 8 stack pages 			//This is why the process table is required
		 				- clear 					//do memory clear in BitMap
		 											//clear sets the valid for the pages to false
		 				- pageTableEntry - valid 	//initialize valid for each page to true when the pages are being created 
		 	Case 3: Last executing thread in a Process
		 				not the last process
		 			Reclaim all memory not reclaimed //all Locks and CVs

		 	//After all of the cases are done, you still need to call current thread finish

	Process Table: populated by Fork and Exec
		2 types of data
			-process specific
				-addrSpace*
				-thread count
			
			-thread specific	//multiple threads in a process table
				-stack page location 
//have different arrays for locks and CVs