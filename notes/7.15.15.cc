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
		2. Create an array of ints in the entity 
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


	Variation #2 - Majority Rules										//Basically everyone votes on who enters next
	 2 New Rules
	 	1. A member can enter a C.S. with N/2 +1 oks
	 	2. A member only sends 1 ok at a time - wait for Release to send next ok
	 	+ Can lose N/2 - 1 members and still function correctly
	 	- SimplePoint of failure - member in C.S. dies



	P4 Part 2 
	 Extension to fully dist. mutual exclusion between multiple servers 
	 	* servers are the group, not clients 
	 	// Part 1 should have no effect on the airport
	 "No" change to clients 
	 	* randomly pick a machine ID for server
	 	* work with 1 to 5 servers
	 	* machine IDs 0,1,2,3,4....
	 	* Add a command line arguement - # of servers and store it into a global int in system.cc/h
	 	//machine IDs will start after
	 	//you can implement this portion in part 1 so that integrating parts 1 and 2 are simpler
	 	//xterm&


	 Every Sever will process every client request 				//total redundancy 
	 	* For this to work, each server must process client requests in the same order 
	 2 Major Tasks:
	 //suggestion: do number 1 first and print out the messeages that each server is getting to make sure that
	 //every server is getting every message
	 	1. When a server gets a client request, the server immediately forwards to all other servers
	 		Add: timestamp of send to the msg 
	 			 machine ID & mailbox number of original server 
	 		Do not lose client machine ID & mailbox number

	 	2. implement Total Ordering
	 	 Data
	 	 	1. Sorted queue of pending client request msgs
	 	 		* messages waiting to be processed
	 	 	by timestamp & forwarding servers machine ID
	 	 		* Nachos List class has Sorted Insert/Remove

	 	 	2. Last timestamp Recieve Table 
	 	 	 	* An array (uses server machine ID as index)			;
	 	 	 	of timestamp values
	 	 	 		- Only highest timestamp


	 	Total Ordering Algorithm	//MUST TEST THAT THE DATA IS CORRECT, MSG QUEUING IS CORRECT, ETC.
	 	1. sever recieves a forwarded msg
	 	2. Extract timestamp and forwarding server machine ID //needed to update the timestamp recieve table
	 	3. Put msg in sorted message queue
	 	4. Update L.T.R table with the info from step 2 
	 	5. Scan timestamp table and get smallest time 
	 	6. Process any msg in our pending msg queue with a timestamp <= step 5 time 