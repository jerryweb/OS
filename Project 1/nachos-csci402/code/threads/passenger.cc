/*This is where all of the functions for the passenger class are defined 
 */
#include "passenger.h"
//for random number
#include "stdlib.h"
#include "time.h"
#include "securityinspector.h"

//#include "system.h"

Passenger::Passenger(int ID, Ticket T, List* bags, Airport* A,
		SecurityInspector** INSPECTORLIST) {
	id = ID;
	queueIndex = 0;
	airport = A;
	ticket = T;
	airline = T.airline;
	luggageList = bags;
	airport = A;
	securityPass = true;
	inspectorList = INSPECTORLIST;
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

Passenger::Passenger(int ID, int qIndex, Airport* AIRPORT) {
	id = ID;
	queueIndex = qIndex;
	airport = AIRPORT;
	securityPass = true;
}

Passenger::Passenger(int ID, int qIndex, Airport* AIRPORT, int airlineCode,
		SecurityInspector** INSPECTORLIST) {
	id = ID;
	queueIndex = qIndex;
	airport = AIRPORT;
	airline = airlineCode;
	securityPass = true;
	inspectorList = INSPECTORLIST;
}

Passenger::Passenger(Airport* AIRPORT) {
	airport = AIRPORT;
}

Passenger::~Passenger() {
}

//----------------------------------------------------------------------
// This function iterates through a given array and sets the location
// of the smallest value and the value itself to the location and 
// minValue variables that are passed in
//----------------------------------------------------------------------
int Passenger::findShortestLine(List** list, bool CISline) { //, //int *location, int *minValue){
	int location = 0;
	int minValue = 0;	//this is the size and location of the smallest line

	if (!CISline) {
		minValue = list[0]->Size();

		for (int i = 0; i < 7; i++) {
			//printf("Size: %d\n", list[i]->Size());
			if (minValue > list[i]->Size()) {
				minValue = list[i]->Size();
				location = i;
			}
		}
		return location;						//Found a line
	}

	else {
		int CIS_ID = airline * 6 + 1;
		minValue = airport->passengerList->Size();
		for (int i = CIS_ID; i < CIS_ID + 5; i++) {
			if (minValue > list[i]->Size()
					&& airport->checkinState[i] != CI_CLOSED) {
				minValue = list[i]->Size();
				location = i;
			}
		}
		return location;						//Found a line
	}

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
	myLine = findShortestLine(airport->liaisonQueues, false);// passenger will find shortest line

	printf("Passenger %d chose liaison %d with a line length of %d\n", getID(),
			myLine, airport->liaisonQueues[myLine]->Size());
	airport->liaisonQueues[myLine]->Append((void *) this);

	airport->liaisonLineLock->Release();  //+++++++++++
	//----airport->liaisonLineLock->Acquire();
	airport->liaisonLock[myLine]->Acquire();  //++++++++
	//----if(airport->liaisonState[myLine] == L_BUSY){						// If the liaison is busy
	//Wait in line
	// add passenger to queue
	//printf("Size: %d\n", airport->liaisonQueues[myLine]->Size());
	//----airport->liaisonLineCV[myLine]->Wait(airport->liaisonLineLock);
	//----airport->liaisonLineCV[myLine]->Wait(airport->liaisonLock[myLine]);   //+++++
	//printf("*******************Passengerline 144***************\n");
	airport->passengerWaitLiaCV[myLine]->Wait(airport->liaisonLock[myLine]);
	//printf("*******************Passengerline 146***************\n");
	//----}
	//----else
	//----	airport->liaisonLineLock->Release();

	//printf("*******************Passengerline 151***************\n");
	airport->liaisonLock[myLine]->Acquire();

	//Give liaison information
	//printf("*******************Passengerline 155***************\n");
	//------airport->liaisonCV[myLine]->Signal(airport->liaisonLock[myLine]);
	airport->liaWaitPassengerCV[myLine]->Signal(airport->liaisonLock[myLine]);
	//printf("*******************Passengerline 158***************\n");
	airport->liaisonLock[myLine]->Release();

	printf("Passenger %d of Airline %d is directed to the airline counter.\n",
			id, ticket.airline);

	//Links to next interaction at the checkin counter
	if (airport->checkInStaffList->Size() > 0)
	CheckIn();
}

void Passenger::Screening() {

	//memeory the current index in screening queue
	//since it may be updated to a different value
	//when being assigned to security inspector
	int oldQueueIndex = queueIndex;

	//ping-pong with Screening Officer
	//correspond to ScreenOfficer C.S.(1)
	airport->screenLocks[queueIndex]->Acquire();
	airport->passengerWaitOfficerCV[queueIndex]->Wait(
			airport->screenLocks[queueIndex]);
	//maybe do something here, at the moment nothing
	airport->screenLocks[oldQueueIndex]->Acquire();
	airport->officerWaitPassengerCV[oldQueueIndex]->Signal(
			airport->screenLocks[oldQueueIndex]);
	airport->screenLocks[oldQueueIndex]->Release();

	//Inspecting();
}

void Passenger::Inspecting() {

	//corresponds to SecurityInspector C.S.(1)
	airport->securityLocks[queueIndex]->Acquire();
	airport->passengerWaitInspectorCV[queueIndex]->Wait(
			airport->securityLocks[queueIndex]);

	if (!securityPass) {
		//corresponds to SecurityInspector C.S.(4)
		airport->securityLocks[queueIndex]->Acquire();
		airport->inspectorWaitPassengerCV[queueIndex]->Signal(
				airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Release();

		//yield 1-5 cycles randomly
		srand(time(NULL));
		int randNum = rand() % 5 + 1;
		for (int i = 0; i < randNum; i++) {
			currentThread->Yield();
		}

		//signal potential inspector waiting on
		//questioning passenger
		airport->securityLocks[queueIndex]->Acquire();
		airport->inspectorWaitQuestioningCV[queueIndex]->Signal(
				airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Release();

		//C.S. to append itself to return Queue
		//since return queue is not shared between inspectors
		//we don't need Condition variables here
		airport->securityLocks[queueIndex]->Acquire();
		airport->returnQueues[queueIndex]->Append(this);
		airport->securityLocks[queueIndex]->Release();

		//corresponds to SecurityInspector C.S.(2)
		airport->securityLocks[queueIndex]->Acquire();
		airport->rePassengerWaitInspectorCV[queueIndex]->Wait(
				airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Acquire();
		airport->inspectorWaitRePassengerCV[queueIndex]->Signal(
				airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Release();

	} else {
		//corresponds to SecurityInspector C.S.(4)
		airport->securityLocks[queueIndex]->Acquire();
		airport->inspectorWaitPassengerCV[queueIndex]->Signal(
				airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Release();
	}
}

void Passenger::SetQueueIndex(int qIndex) {
	queueIndex = qIndex;
}

void Passenger::SetSecurityPass(bool pnp) {
	securityPass = pnp;
}

bool Passenger::GetSecurityPass() {
	return securityPass;
}

void Passenger::CheckIn() {
	printf("*********Passenger %d Class starting ck in \n",id);
	airport->checkinLineLock[airline]->Acquire();
	printf("***************Passenger 258************************\n");
	// Find the shortest line to get into. Default is executive.
	int checkInLine = airline * 6;
	if (!ticket.executive) {
		//airport->checkinQueuesLock->Acquire();
		checkInLine = findShortestLine(airport->checkinQueues, true);
		printf(
				"Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n",
				id, airline, checkInLine,
				airport->checkinQueues[checkInLine]->Size());
	} else {
		printf(
				"Passenger %d of Airline %d is waiting in the executive class line\n",
				id, airline);
	}
	airport->checkinQueues[checkInLine]->Append((void *) this);
	//airport->checkinQueuesLock->Release();
	airport->checkinLineCV[checkInLine]->Wait(
			airport->checkinLineLock[airline]);
	airport->checkinLineLock[airline]->Acquire();
	printf("Passenger %d of Airline %d was informed to board at gate %d\n", id,
			airline, boardingPass.gate);
	airport->checkinLineLock[airline]->Release();

	//linking to screening
	airport->screenQueuesLock->Acquire();

	int shortest = 0;		//shortest line's id
	int minimumSize = -1;		//for comparsion in the following loop

	//find the shortest line
	for (int i = 0; i < 3; i++) {
		if (minimumSize < 0 || minimumSize > airport->screenQueues[i]->Size()) {
			minimumSize = airport->screenQueues[i]->Size();
			shortest = i;
		}
	}
	airport->screenQueues[shortest]->Append(this);
	airport->screenQueuesLock->Release();

	Screening();
}

void Passenger::FindShortestScreenLine() {

}

void Passenger::IWantToFly() {
	findShortestLiaisonLine();
	//CheckIn();
	Screening();
	Inspecting();
}
