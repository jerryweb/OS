Project 3 -3 parts
	parts 1 & 2 - Demand Page virtual memory 	//dont preload anything (no bitmap finds in the constructor; also we can no longer do a new page table each time)
	part 3 - networking - client server			//should work even if parts 1 & 2 are not
		distributed locks/CVs

	An Approach to Parts 1 & 2
		* compile and run in VM directory 		//may make changes to the files in the other directories (user programs still go into test)
			- until part 3 is ready				//you could do part 3 with project 2 code
				-move to network directory

	VM directory Makefile 						//when running from userprog, nachos used the page table
		* nachos will now use the TLB not the page table	//no bitmap finds for the page table
			-> page fault exception 			// need to add an else if(page fault exception) in the exception.cc file around the switch case
				if(which === SyscallException){

				}
				//add this statement 
				else if(which == PageFaultException){
					//part 1 and 2 code; suggest using a seperate function so that this is not 100s of lines long
				}
				else{

				}
		* anywhere you see:
		1.	machine->pageTable = pageTable;		//comment this out (should be the first thing you do)
		
		Step 1 - Populate the TLB from the pageTable
			Project 2:  Infinite memory
						Still preload
			On a page fault exception: use the page table to Populate the TLB
			1. what is the vpn containing the needed virtual address?
				* the needed address is in register 39 - Bad vAddrReg
					- divided by PageSize => page table index
					currentThread->space->pageTable
				* copy the pageTable data into TLB //TLB is an array based entry table just like the pagetable
			2. Where to copy into the TLB?		//Lock doesn't work very well here
				machine->TLB
				* array of TranslationEntry
				* size 4
				* Method: treat it like a circular 'queue'
				* Create a variable - int currentTLB = 0; //this is going to be a system variable located in system.h
					- wrtie to the index
					currentTLB = (currentTLB++) % TLBSize; //When you copy to the entry to the TLB, you must copy the parameters NOT the pointer
				/*anytime you do anything with the TLB, disable interrupts*/
				disable interrupts
				// populate TLB
				restore interrupts


				On a context switch
					- invalidate the TLB //throw out the TLB info
						* set the valid bit to false
							//this ca go in the one of the following four places; if put in one of the first two, put it below the existing code
							//Don't put a lock here
							for(int i = 0; i <TLBSize; i++){
								machine->TLB[i].valid = false;
							}
					//AddrSpace
					SaveState
					RestoreState
					//Thread
					SaveUserState
					RestoreUserState

		Step 2 - Implement IPT 					//essentially a map of physical pages
			Can be an  array					//more realistically you may want to use the stl hash map
				* have numPhysPages entries 
			Still project 2 assumptions
			Add IPT population code to where pagetable is set 
				* whereever you do a memMap->Find //idnex to IPT
					- AddrSpace constructor
					- Fork Syscall
					- Exit 
						* on a clear, set the valid bit to false in the IPT 

			On a Page Fault Exception (PFE);, use IPT 				

			Issue /*when using array*/: on a PFE, we have VPN, IPT needed by PPN
				Do a sequential search of 3 values 
					- valid bit is True
					- match the vpn
					- match AddrSpace* 			//AddrSpace pointer is useful here if you don't have a unique number. Need to match all three 
						//This doesn't exist 

				If all 3 match, copy that index location to TLB 	//Restart the same command because we haven't run the instruction yet
	
			Two Choices //Because you CAN'T change TranslationEntry 
			1. Create a new Class that inherits from TranslationEntry
				* add AddrSpace* and Process ID
			2. Copy/Paste TranslationEntry code to a new CLass and then add the AddrSpace* and Process ID
			/**/
			Result: all the project 2 code should run
			//MAKE SURE YOU RUN YOUR TESTS AFTER EVERY STEP TO MAKE SURE EVERYTHING WORKS!!!!

		Step 3 - Do not preload anything
			still lots of memory
			//any place you were previously doing a find(), comment this out

			Can now get an IPT miss
				Solution: move the preloading code & IPT populationcode from step 2 => into P.F.E. handling code

			AddrSpace constructor
				for(i = 0 to numPhysPages){
					int ppn = bitMap->Find()		//Comment this out
					.
					.
					.
					pageTable[i].valid = false;
					.
					.
					.
					//executable->ReadAt()

				}

			Similar Changes in Fork IF you are making a new pageTable each time

			On a PFE from step 2		//This is from step 2
				int ppn = 1;
				for (i =0 to numPhysPages){
					if(/*check 3 values */){
						ppn = i;
						break;
					}
				}

				/*Step 3 goes here*/
				if (ppn == 1){
					ppn = handleIPTMiss(vpn);
				}


				Populate the TLB from IPT 		//also from step 2