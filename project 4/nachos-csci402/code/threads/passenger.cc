/*This is where all of the functions for the passenger class are defined 
 */
#include "passenger.h"
//for random number
#include "stdlib.h"
#include "time.h"
#include "securityinspector.h"

Passenger::Passenger(int ID, int airlineCode, Ticket T, List* bags,
		Airport* A) {
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
		bool Securityline) {
	int location = 0;
	int minValue = -1;	//this is the size and location of the smallest line

	if (!CISline && !Screenline && !Securityline) {
		minValue = list[0]->Size();

		for (int i = 0; i < 7; i++) {
			if (minValue < 0 || minValue > list[i]->Size()) {
				minValue = list[i]->Size();
				location = i;
			}
		}
	}

	else if (CISline && !Screenline && !Securityline) {
		int CIS_ID = airline * 6 + 1;
		minValue = airport->passengerList->Size();
		for (int i = CIS_ID; i < CIS_ID + 5; i++) {
			if (minValue < 0
					|| minValue > list[i]->Size()
							&& airport->checkinState[i] != CI_CLOSED) {
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

	//start first C.S.
	airport->liaisonLineLock->Acquire();
	myLine = findShortestLine(airport->liaisonQueues, false, false, false);	// passenger will find shortest line

	printf("Passenger %d chose liaison %d with a line length of %d\n", id,
			myLine, airport->liaisonQueues[myLine]->Size());
	airport->liaisonQueues[myLine]->Append((void *) this);
	//wait liaison to start service
	airport->liaisonLineCV[myLine]->Wait(airport->liaisonLineLock); //ending first C.S.

	//start second C.S.
	airport->liaisonLock[myLine]->Acquire();
	airport->liaisonCV[myLine]->Signal(airport->liaisonLock[myLine]);
	//wait liaison to give direction
	airport->liaisonCV[myLine]->Wait(airport->liaisonLock[myLine]);

	airport->liaisonLock[myLine]->Acquire();
	//signal liaison that I aknowledge the direction
	airport->liaisonCV[myLine]->Signal(airport->liaisonLock[myLine]);
	airport->liaisonLock[myLine]->Release();  //end second C.S.


	printf("Passenger %d of Airline %d is directed to the airline counter.\n",
			id, ticket.airline);

	//Links to next interaction at the checkin counter
	if (airport->checkInStaffList->Size() > 0)
		CheckIn();

}

void Passenger::CheckIn() {

	//start 1st C.S.
	airport->checkinLineLock[airline]->Acquire();

	int myLine;

	if (ticket.executive) {
		myLine = airline * 6;
		printf("Passenger %d of Airline %d is waiting in the executive class line\n",
				id, airline);
	} else {
		myLine = findShortestLine(airport->checkinQueues,true,false,false);
        printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n",
				id, airline, myLine,
				airport->checkinQueues[myLine]->Size());
	}
	airport->checkinQueues[myLine]->Append(this);
	//wait check-in to start service to me
	airport->checkinLineCV[myLine]->Wait(airport->checkinLineLock[airline]); //end 1st C.S.

	airport->checkinLock[queueIndex]->Acquire();
	airport->checkinCV[queueIndex]->Signal(airport->checkinLock[queueIndex]);
	//wait for check-in to do check-in
	airport->checkinCV[queueIndex]->Wait(airport->checkinLock[queueIndex]);

	airport->checkinLock[queueIndex]->Acquire();
	//sent acknowledgement to check in
	airport->checkinCV[queueIndex]->Signal(airport->checkinLock[queueIndex]);
	airport->checkinLock[queueIndex]->Release();

	//go to next procedure
	if (airport->screenOfficerList->Size() > 0)
    {
		Screening();
    }
}

void Passenger::Screening() {
	int myLine;

	//start 1st C.S.
	airport->screenQueuesLock->Acquire();
	myLine = findShortestLine(airport->screenQueues,false,true,false);
	airport->screenQueues[myLine]->Append(this);

	//wait officer to check me
	airport->screenQueuesCV[myLine]->Wait(airport->screenQueuesLock); //end of 1st C.S.

	//start 2nd C.S.
	airport->screenLocks[myLine]->Acquire();
	airport->screenCV[myLine]->Signal(airport->screenLocks[myLine]);
	// Give bag to officer
	printf("Passenger %d gives the hand-luggage to screening officer %d\n",
			id, myLine);

	//wait officer check
	airport->screenCV[myLine]->Wait(airport->screenLocks[myLine]);

	//tell officer I acknowledge I'm ok to proceed
	airport->screenLocks[myLine]->Acquire();
	airport->screenCV[myLine]->Signal(airport->screenLocks[myLine]);
	airport->screenLocks[myLine]->Release();   //end 2nd C.S.

	//proceed to security inspecting
	if (!airport->securityInspectorList->IsEmpty())
		Inspecting();
}

void Passenger::Inspecting() {
	srand(time(NULL));

	//start 1st C.S.
	airport->securityQueuesLock->Acquire();
	airport->securityQueues[queueIndex]->Append(this);
	printf("Passenger %d moves to security inspector %d\n", id, queueIndex);
	airport->securityQueuesCV[queueIndex]->Wait(airport->securityQueuesLock); //end 1st C.S.

	//start 2nd C.S.
	airport->securityLocks[queueIndex]->Acquire();
	airport->securityCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
	//wait for security check
	airport->securityCV[queueIndex]->Wait(airport->securityLocks[queueIndex]);

	airport->securityLocks[queueIndex]->Acquire();

	//if I failed the security test
	if (!securityPass) {
		//send security my aknowledgement
		airport->securityCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
		printf("Passenger %d goes for further questioning\n", id);
		airport->securityLocks[queueIndex]->Release();

		//yield random cycles
		int randNum = rand() % 10 + 1;
		for (int i = 0; i < randNum; i++) {
			currentThread->Yield();
		}

		//start 3rd C.S. when I back from questioning
		airport->securityQueuesLock->Acquire();
		airport->returnQueues[queueIndex]->Append(this);
		printf(
				"Passenger %d comes back to security inspector %d after further examination\n",
				id, queueIndex);
		airport->securityQueuesCV[queueIndex]->Wait(airport->securityQueuesLock); //end 3rd C.S.
		//start 2nd C.S. again
		airport->securityLocks[queueIndex]->Acquire();
		airport->securityCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
		airport->securityCV[queueIndex]->Wait(airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Acquire();
	}

	//send security my aknowledgement
	airport->securityCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
	airport->securityLocks[queueIndex]->Release();  //end 2nd C.S.

	//add passenger himself to boarding queue
	airport->boardingLock[airline]->Acquire();
	printf("Passenger %d of Airline %d reached the gate %d\n", id,
			airline, airline);
	airport->boardingQueue[airline]->Append(this);

	printf("Passenger %d of Airline %d boarded airline %d\n", id, airline,
			airline);

	//wait for boarding announcement
	airport->boardingCV[airline]->Wait(airport->boardingLock[airline]);
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

