/*This is where all of the functions for the passenger class are defined 
 */
#include "passenger.h"
//for random number
#include "stdlib.h"
#include "time.h"
#include "securityinspector.h"

Passenger::Passenger(int ID, int airlineCode, Ticket T, List* bags, Airport* A) {
	id = ID;
	airline = airlineCode;
	airport = A;
	ticket = T;
	luggageList = bags;
	airport = A;
	securityPass = true;
	isBoarding = false;
	queueIndex = 0;
}

// Main constructor for the passenger. 
Passenger::Passenger(int ID, List* bags, Ticket T, Airport* A) {
	id = ID;
	airport = A;

	luggageList = bags;

	ticket = T;
	securityPass = true;
	airline = -1;
}
// Limited constructors for tests
Passenger::Passenger() {
}
Passenger::Passenger(int ID) {
	id = ID;
}
Passenger::Passenger(int ID, Airport* a) {
	id = ID;
	airport = a;
	securityPass = true;
}
Passenger::Passenger(int ID, Ticket t) {
	id = ID;
	ticket = t;
}
Passenger::Passenger(int ID, Ticket t, int airlineCode, Airport* a) {
	id = ID;
	ticket = t;
	airline = airlineCode;
	airport = a;
}

Passenger::Passenger(Airport* AIRPORT) {
	airport = AIRPORT;
}

Passenger::Passenger(int ID, Airport* A, BoardingPass bp) {
	id = ID;
	airport = A;
	boardingPass = bp;
	securityPass = true;
	isBoarding = false;
	queueIndex = 0;
}

Passenger::~Passenger() {
}

//----------------------------------------------------------------------
// This function iterates through a given array and sets the location
// of the smallest value and the value itself to the location and 
// minValue variables that are passed in
//----------------------------------------------------------------------
int Passenger::findShortestLine(List** list, bool CISline, bool Screenline,
		bool Securityline) 
{
	int location = 0;
	int minValue = airport->passengerList->Size();

	if (!CISline && !Screenline && !Securityline) {
		minValue = list[0]->Size();

		for (int i = 0; i < 7; i++) {
			if ((minValue < 0 || minValue > list[i]->Size()) &&
                airport->liaisonState[i] != L_NONE)
            {
				minValue = list[i]->Size();
				location = i;
			}
		}
	}

	else if (CISline && !Screenline && !Securityline) {
		int CIS_ID = airline * 6 + 1;
		for (int i = CIS_ID; i < CIS_ID + 5; i++) {
			if ((minValue < 0 || minValue > list[i]->Size()) && 
				(airport->checkinState[i] != CI_CLOSED || airport->checkinState[i] != CI_NONE))
            {
				minValue = list[i]->Size();
				location = i;
			}
		}
	}

	else if (!CISline && Screenline && !Securityline) {
		for (int i = 0; i < airport->screenOfficerList->Size(); i++) {

			if (minValue < 0 || minValue > list[i]->Size()) {
				minValue = list[i]->Size();
				location = i;
			}
		}
	} else if (!CISline && !Screenline && Securityline) {
		for (int i = 0; i < airport->securityInspectorList->Size(); i++) {
			if (minValue < 0 || minValue > list[i]->Size()) {
				minValue = list[i]->Size();
				location = i;
			}
		}
	}
    
	return location;
}

//----------------------------------------------------------------------
// This is the first version of the function for the passenger to find 
// the shortest liaison line in the Airport. An array of liaison line 
// sizes is passed to the passenger and iterated to find the min length
// If all lines have the same value, the passenger should enter the first
// line. The Passenger will then get in that queue and wait to be helped.
// Once he has been helped by the corresponding liaison, a print statement 
// describing the ID and airline of the passenger should be printed.
//----------------------------------------------------------------------
void Passenger::findShortestLiaisonLine() {
	int myLine = 0;
    
	airport->liaisonLineLock->Acquire();
	myLine = findShortestLine(airport->liaisonQueues, false, false, false);// passenger will find shortest line

	printf("Passenger %d chose liaison %d with a line length of %d\n", id,
			myLine, airport->liaisonQueues[myLine]->Size());
	airport->liaisonQueues[myLine]->Append((void *) this);
	if (airport->liaisonState[myLine] == L_BUSY) {	// If the liaison is busy
		//Wait in line
		// add passenger to queue
		airport->liaisonLineCV[myLine]->Wait(airport->liaisonLineLock);
	} else
		airport->liaisonLineLock->Release();

	airport->liaisonLock[myLine]->Acquire();

	//Give liaison information
	airport->liaisonCV[myLine]->Signal(airport->liaisonLock[myLine]);
	//wait for liaison confirmation
	airport->liaisonCV[myLine]->Wait(airport->liaisonLock[myLine]);


	printf("Passenger %d of Airline %d is directed to the airline counter.\n",
			id, airline);

	//Links to next interaction at the checkin counter
	if (airport->checkInStaffList->Size() > 0)
		CheckIn();

}

void Passenger::CheckIn() {
	airport->checkinLineLock[airline]->Acquire();
    // Find the shortest line to get into. Default is executive.
	int checkInLine = airline * 6;
	if (!ticket.executive) {
		checkInLine = findShortestLine(airport->checkinQueues, true, false, false);
        printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n",
				id, airline, checkInLine,
				airport->checkinQueues[checkInLine]->Size());
	} else {
		printf("Passenger %d of Airline %d is waiting in the executive class line\n",
				id, airline);
	}
	airport->checkinQueues[checkInLine]->Append((void *) this);
	airport->checkinLineCV[checkInLine]->Wait(airport->checkinLineLock[airline]);

	if (airport->screenOfficerList->Size() > 0)
    {
		Screening();
    }
}

void Passenger::Screening() {
	int myLine = 0;
	airport->screenQueuesLock->Acquire();
	//find the shortest line of screening officers
	myLine = findShortestLine(airport->screenQueues, false, true, false);

	//Append myself to the line
	airport->screenQueues[myLine]->Append((void *) this);
	printf("Passenger %d is getting into Screening officer %d line\n", id, myLine);
	//if current screen officer is busy then wait
	if (airport->screenState[myLine] == SO_BUSY) {		
		airport->screenQueuesCV[myLine]->Wait(airport->screenQueuesLock);
		// airport->screenLocks[myLine]->Acquire();
		// airport->screenFreeCV[myLine]->Signal(airport->screenLocks[myLine]);
		// airport->screenLocks[myLine]->Release();
	}
	else{
		// airport->screenQueuesCV[myLine]->Wait(airport->screenQueuesLock);
		airport->screenQueuesLock->Release();
	}

	airport->screenLocks[myLine]->Acquire();

	// Give bag to officer
	printf("Passenger %d gives the hand-luggage to screening officer %d\n", id, myLine);
	airport->screenCV[myLine]->Signal(airport->screenLocks[myLine]);
	airport->screenCV[myLine]->Wait(airport->screenLocks[myLine]);

	

	//proceed to security inspecting
	if (!airport->securityInspectorList->IsEmpty())
		Inspecting();
}

void Passenger::Inspecting() {
	srand(time(NULL));
	int myLine = queueIndex;
	airport->securityQueuesLock->Acquire();
	//myLine = queueIndex;

	//add passenger to the security line
	//airport->securityQueues[myLine]->Append((void *) this);

	printf("Passenger %d moves to security inspector %d\n", id, myLine);

	//if security inspector is free signal him
	if (airport->securityState[myLine] == SC_FREE) {
		airport->securityLocks[myLine]->Acquire();
		airport->securityFreeCV[myLine]->Signal(airport->securityLocks[myLine]);
		airport->securityLocks[myLine]->Release();
	}

	//wait for inspector to do security check
	airport->securityQueuesCV[myLine]->Wait(airport->securityQueuesLock);

	//if pass
	if (! securityPass) {
		//If fail the check, go to questioning
		printf("Passenger %d goes for further questioning\n", id);
		//yield random cycles
		int randNum = rand() % 5 + 1;
		for (int i = 0; i < randNum; i++) {
			currentThread->Yield();
		}

		//after questioning go to the return queue of the same inspector
		airport->securityQueuesLock->Acquire();
		airport->returnQueues[myLine]->Append((void*) this);

		printf("Passenger %d comes back to security inspector %d after further examination\n", id, myLine);

		//wake up on break inspector
		if (airport->securityState[myLine] == SC_FREE) {

			airport->securityLocks[myLine]->Acquire();
			airport->securityFreeCV[myLine]->Signal(airport->securityLocks[myLine]);
			airport->securityLocks[myLine]->Release();

		}

		//wait for inspector to clear my boarding status
		airport->returnQueuesCV[myLine]->Wait(airport->securityQueuesLock);
    }
    
    //tell inspector I acknowledge I can board now
    airport->securityLocks[myLine]->Acquire();
    airport->securityFinishCV[myLine]->Signal(airport->securityLocks[myLine]);
    airport->securityLocks[myLine]->Release();

    //add passenger himself to boarding queue
    airport->boardingLock[airline]->Acquire();
    printf("Passenger %d of Airline %d reached the gate %d\n", id, airline, airline);
    airport->boardingQueue[airline]->Append(this);

    //wait for boarding announcement
    airport->boardingCV[airline]->Wait(airport->boardingLock[airline]);
    printf("Passenger %d of Airline %d boarded airline %d\n", id, airline, airline);
    currentThread->Finish();
}

void Passenger::SetQueueIndex(int qIndex) {
	queueIndex = qIndex;
}

void Passenger::SetSecurityPass(bool pnp) {
	securityPass = pnp;
}

void Passenger::SetBoarding() {
	isBoarding = true;
}

bool Passenger::GetBoarding() {
	return isBoarding;
}

bool Passenger::GetSecurityPass() {
	return securityPass;
}

