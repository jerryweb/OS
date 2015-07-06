int availChairs = 10
int barberState = BUSY;

Locks:  chairLock
	    barberLock

CVs: 	waitingCV
		barberState

// Barber

	while(true){
		chairLock->Acquire();
		if(availChairs ! = 10)
			waitingCV->Signal();
		else 
			barberState =  AVAIL;

		baberLock->Acquire();
		chairLock->Release();
		barberCV->Wait(barberLock); //Wait fo Cust

		//Cut hair
		barberCV->Signal(	);
		barberCV->Wait(		);
		barberLock->Release();
}

//Customer

	chairLock->Acquire();
	if(barberState != AVAIL){
		if(availChairs > 0){
			availChairs--;
			waitingCV->Wait(	);
			availChairs++;
		}
	}
	else
		barberState = BUSY;


	chairLock->Release();
	barberLock->Acquire();
	S 
	W 
	S

//----------------------------------------------------------------------------------------
NetWorking:
	Remote Procedure Calls (RPCs);
	 Based on the idea of calling a function
	 	* jump to a section of code
	 	* execute the code
	 	* return where you jumped from
	 		- maybe return a value
	 	//networking does this, however, it will jump to a function on a different machine

	 For RPCs, the code is on the another computer

	How to implement?
	 1. Require user program to explicitly issue Send/Recieve  requests
	 	+ Easy on OS. Will use 2 new syscalls
	 	- Requires user program to 'know' server location & protocol	//Limit flexibility
	 	//if you wanted to move the server, all the clients would have to be modified

	 2. Hide the fact that 'work' is being done remotely
	 	* no networking code in the user program
	 	* NetWork calls are in a 'middle' layer OR are in the OS kernel

	 	How to do this?
	 	 Use 'stubs' programs to do networking
	 	 	* client stub: issues requests and recieves replies 
	 	 	* server stub: recieve requests and sends replies


Project 3 Part 3
 Convert the Lock & CV syscalls to be RPCs
 //Client side
 Client App: Nachos user programs
 Client Stub: Nachos kernel code
 //Server side
 Server Stub \ __  Server kernel
 Server App  /		function
 //The server app does not run user programs 

	 Client Nachos //from project 2
	 	Exception Handler
	 	1. Determine the syscall number
	 	2. Get the parameters
	 	3. Validate the parameters
	 	4. Do the work
	 	5. return a value

	 	/****For Project 3****/
	 	1. 
	 	2.
	 	3. some of the validation cn be done on the client side
	 		Create a request message & send to server
	 		Wait for reply
	 	5.

	 Server Nachos
	 	while(true){
	 		1. Wait for a request message
	 		2. Determine the request type			//need to use the similar method as the syscall numbers to determine which request it is
	 			Put the request number @ the start of each message
	 		3. Parse the rest of the message
	 		****easiest way to parse the message is to use the Stringstream
	 		//at the top 
	 		#include <sstream>
	 		using namespace std;

	 		int regType;

	 		Stringstream ss;
	 			ss<<msg;
	 			ss>>regType;

	 		char[] msg - 12_hello

	 		string name;
	 		ss>>name;

	 		4. Validate the parse
	 		5. Do the work
	 		6. Create & send the reply message maybe
	 	}


	Server - Acquire(on Server);
		Server has the Lock
		 * it is busy 

		from Project 2: lockTable[i]->Acquire(); 	//can't use locks to put the server to sleep 
		 	Cannot use real locks nor CVs on server //need something that acts like locks/CVs but doesn't put the server to sleep

	Server Lock
		state - avail/busy
		owner - machine ID 
				mailbox number
		wait queue - reply msg

	Server CV
		wait queue
		lockUsed -  index to lock in ServerLock table	//this must be used because we 
														//cannot use lock ponters
		*** NO PROCESS OWNER

	//For locks
	Only send a reply msg - when a thread can properly proceed 
		Acquire:
		- lock avail - change state to BUSY and send reply
		- lock not avail - queue reply msg

		Release:
		- no waiting - change state to AVAIL and send reply
		- waiting - get reply msg from queue and send it
				  - change the owner

	//For CVs
	Wait(int lock, int CV){
	- queue reply msg
	- Release(int lock)							//similar to the ther release functions created
		have a release msg function
	}

	Signal		;
	- remove 1 reply msg from the wait queue
		* Acquire lock for this user program
		//Acquire will send the reply
	-reply to sender of the Signal


	//from project 1
	void Condition::Wait(Lock* locks){
		lock->Release();
		currentThread->sleep();
		lock->Acquire();
	}


User Program A
	//a is the index number
	a = CreateLock("lock1",5);				//if the name exists just return the index value of the lock
											//else just create a new one
											//This allows both a and b to share 
	Acquire(a);
	Printf(.....);

User Program B 
	//b is the index number
	b = CreateLock("lock1",5);
	Acquire(b);

Testing: will need 6 windows for full testing 
	can use xtern& on x11
	//first window
	nachos -m -server
	//rest of the windows
	nachos -m1 -x ...
	nachos -m2 -x ...
	.
	.
	.

	sever machineID = 0 //will be 0
	clients 1 - 5
	mailbox number != 0	//cannot be 0


Memory:
	Scenario
	 Have an IPT miss and Memory is full. We pick a page to evict
	 The selected page belongs to the current process
	 AND it is in the TLB

	Solution: on an eviction, if evicted page belongs to me, look for this VPN in the TLB
	 1st. /*before eviction*/ if valid: propagate the dirty bit to the IPT 
										invalidate the TLB
	 THEN: start page eviction

	Testing/Debug Step 4
	- make numPhysPages large
		* problem could be that there is no swapping
		* no -rs 
		* then use -rs while numPhysPages is still large			//If the code works, then this is a race condition
																	//The swapping code is wrong if this happens
	- If swapping code is wrong:
		* only with Exec, probably updating wrong page table
		* page table updated wrong
			swap vs executable
			offest correct?
		* IPT: vpn vs ppn