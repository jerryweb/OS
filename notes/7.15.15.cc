HandlePageFault
	IPTLock->Acquire
	//searchhing IPT
	- set USE true
	IPTLock->Release

	//handleIPTMiss if it cannot be found

HandleIPTMiss

	ppn = HandleMemoryFull;
	read (load) from disk;

HandleMemoryFull
	* Pick a page to evict
	* set USE in IPT to true
	* Figure out if must swap out
	  - Yes: get swap location 	swapLock->Acquire	//anytime you do an acquire or release, 
	  								Find			//you have 2 chances of context switching
	  							swapLock->Release
	* writing to swap file
	* Update the page table with swap info
	//cannot just turn interrupts off for these steps 


Project 4: Distributed Airport - Part 1
	- Regular airport 
	- Exec all "threads"
	- Split up P2 into multiple user progs		//will now have a liaison.c, passenger.c, etc.
		* Default number of mailboxes is 10 	//count is 10, will need to change this
			* In Initialize(); in system.cc
			... new PostOffice(10);
		1. client - Exec all Passengers
		2. client -  Liaison & CheckIn

	Change Thread class 
	 global int & swapLock
	 Add mailbox int to Thread

	Memory Management
	- make NumPhysPages big 

	Nachos Network Warning 
	 in PostOffice::Send
	 	//near the top of send
	 	usleep(50);		// the u stands for micro... will hold the process by 50 micro secs
	 	//make the number a bit larger if one instance seems to stop

	Create Lock/CV/MV
		Suggest creating a create.h file with a function that will do all the creates
		void doCreates(){

		}

	passenger.c
	 #include "create.h" 

	 int main(){
	 	doCreates();
	 		.
	 		.
	 		.
	 }

	Arrays of Locks/CVs/MVs?
	 2 choices
		1. Create new Creates that create arrays 
		2. Create an array of ints in th entity 
			* use a for loop to do creates 
			// 5 CVs for the LiaisonLineCV
			* LiaisonLineCV & append for loop index
			LiaisonLineCV 0
			LiaisonLineCV 1
				.		  2
				.		  3
				.		  4

			// a good test is to print out the ints that are in the array

	Distributed Mutual Exclusion - Part 2
	 3 Basic Ways
	 	- Centralized approach
	 	- Fully Distributed
	 		* no single server
	 	- Token Ring

	 Centralized
	 * 1 mutual exclusion server
	 * It handles all client requests
	 * A request comes to enter a C.S.
	 	- If no client in that C.S., reply with OK msg
	 	- If a client in that C.S., queue the request
	 * Upon exit of a C.S., client sends Release to msg to server
	 	- If a client waiting on that C.S., send OK msg
	 Requirement: Each client needs a unique ID
	 // This is correct and fair (as long as new requests are placed in the back)
	 // Single point of failure... BAD
	 // Scalability Problems arise

	 Fully Distributed
	 * no central server(s)				;
	 	- All entities work together to make decisions 
	 	Key: all entities make the same decisions in the same order		//Total Ordering of Events
	 	Requirements:
	 	1. Reliable communication
	 	2. Globally unique IDs
	 	3. Total ordering of events 
	 		* An event, in a Distributed system, is a Send/Recieve 
	 		* All entities must agree on the Send order of msgs -> use timestamps in msgs
	 		* msgs will be recieved in different order

	For part 2, we will be using 1 to 5 servers 

	Algorith
	1. A member sends a request msg to all other members 
		msg contains: critical section identifier
					  timestamp
					  ID of requesting member
	
	2. A member processing a request can be in 1 of 3 states
		a. member is not in that C.S.,  and has no pending request for that C.S.
		b. member is in that C.S. 
		c. member is not in that C.S., but has its own pending request for that C.S.
	
	3. How/When to respond
		a. Send ok 
		b. Queue the request; Upon C.S. exit, send ok to all queued requests
		c. Compair both request msg timestamps
			* If my timestamp earlier - like b
							  later   - like a
			Rule: no ties 
				* Use ID - lower number wins 
		C.S. Entry Rule: get ok from all 

	Worst Case: All members request - timestamps same