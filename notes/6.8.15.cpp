int LiaisonState[];		//array of states for the LiaisonState
	//2 values: busy, free
	//initialize the array so that all the liaisons are busy 
	//this will make sure that the passengers wait for the liaison to start his thread even if the passengers
	//are initialized first 

LineLock
LineCV[]
LineCount[]


Passenger
	int myLine
	LineLock->Aquire();
	for(){
		//find a line
	}
	//found my line
	if(LiaisonState[myLine] == BUSY){	//if(LineCount[myLine] > 0)
		//Wait in line
		lineCount[myLine]++;
		LineCV[myLine]->Wait(	);
		//lineCount[myLine]--;			//This is if you are not using states for the liaison to check if for the line in the above if statement 
	}
	
	LineLock->Release();


/*you will need a state variable for things that can go on break*/

Liaison	
	LineLock->Aquire();
	//no advantage to setting the liaison to free for the first iteration
	if(lineCount[myLine] > 0){
		LineCV[myLine]->Signal();
		lineCount[myLine]--;			//this does not produce a race condition because he is set to busy in the same Critical Section
		LiaisonState[myLine] = BUSY;
	}

	else{
		LiaisonState[myLine] = FREE;
	}

	liaisonLock[myLine]->Aquire();
	LineLock->Release();
	LiaisonCV[myLine]->Wait(	);


int cisState[];
	- 	BUSY
	-	FREE
	-	onBREAK

Check-in staff						//start everyone off in the BUSY state
	while(true){					//same with cargo handlers
		cisLineLock->Aquire();
		bool havePassenger = false;
		//check execline length
		//if no exec passengers, check my econ line
		//Theese can be handled by bool havePassenger = false;
		if(!havePassenger){		//go on break
			cisState[myLine] = onBREAK;
			cisOnBreakCV[myLine]->Wait(	);
			cisState[myLine] = 	BUSY;
			cisLineLock->Release();
			//cisLineCV[myLine]->Signal(	);
			continue;
		}

		//if all checkin staff are asleep and there is a passenger in the executive line, then do a broadcast

		cisCV[myLine]->Aquire();
		cisLineLock->Release();




		cisCV[myLine]->Release();
	}



	CIS 
	-each have their own line 


	Cargo Handlers								-starting # of C.H.
		-one line per airline
		-Broadcast from mannagers to wake them up
		-on break

		if bagCount > 0 && all C.H. on break 	-chonBreakCount
			then Broadcast

		chBreakCV			//can't have one CV for break; need to have an array 

// Manager NEVER GOES TO SLEEP 
Airport Manager
	while(true){
		lock1->Aquire();
			//do task
		lock1->Release();

		lock2->Aquire();
			//do task
		lock2->Release();
			.
			.
			.
		for(int i = 0; i < 100 (or 1000); i++){ //this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();
		}
	}









int passSecurtiy[maxPassengers];	//only shared between the Screeing Officer and Security Inspector

Screening Officer
	
	SO[myLine]->Aquire();
		//Generate random PASS/FAIL
		// -get passenger's index number (index in the array of the total number of passengers) - myPassenger
		//find a line for this passenger w/SI
			// -find shortest line
			// -give that line # to passenger
			// -give PASS/FAIL to the proper Security Inspector
		passSecurtiy[myPassenger] = PASS/FAIL;


	soCV[myLine]->Wait(	);


Passenger
	while(true){
		//start of S.I. code
		didIPass = SIPassFail[siLine];	
		siLock[siLine]->Release();
		SO[soLine]->Aquire();
			//give bag to S.O.
		soCV[soLine]->Wait(	);

		//passenger waits for PASS/FAIL
							
			if(!didIPass){					//This statement is not with the Security Inspector; outside of S.I. CV
				for(){
					currentThread->Yield();
				}
				continue;
			}

			else{
				break;
			}

		siCV[siLine]->Wait(	);
	}



Screening Inspector 

	siLineLock->Aquire();
			.
			.
			.
		getPassenger #
	//passenger fails
		-tell them SIPassFail[myLine] = PASS/FAIL;
		-set their PASS/FAIL to PASS



Passenger version 2
	bool didIFail;
	while(true){
		SiLock[mySi]->Aquire();
			//give passenger #
			siCV[mySi]->Signal(	);
			siCV[mySi]->Wait(	);

			didIPass = SIPassFail[mySi];
			siCV[mySi]->Signal(	);
		siLock[mySi]->Release();
		if(!didIPass){ continue; }

		else{ break; }
	}



Screening Inspector version 2

	siLock[myLine]->Aquire();
	siLineLock->Release();
		siCV[myLine]->Wait(	);			//wait for passengr #
		//Generate PASS/FAIL
		passSecurtiy[passNow] = PASS/FAIL;
		SIPassFail[myLine] = PASS/FAIL;

		siCV[myLine]->Signal(	);
		siCV[myLine]->Wait(		);		//wait for passenger to read value
		passSecurtiy[passNow] = PASS;
		siCV[myLine]->Signal(	);


submission: /home/scf-22/csci350/bin/