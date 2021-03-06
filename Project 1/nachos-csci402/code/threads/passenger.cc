/*This is where all of the functions for the passenger class are defined 
*/
#include "passenger.h"
//for random number
#include "stdlib.h"
#include "time.h"
#include "securityinspector.h"

//#include "system.h"

Passenger::Passenger(int ID,int qIndex,int airlineCode,Ticket T,List* bags,Airport* A,SecurityInspector** INSPECTORLIST) {
	id = ID;
	queueIndex = qIndex;
	airline = airlineCode;
	airport = A;
	ticket = T;
	luggageList = bags;
	airport = A;
	securityPass = true;
	inspectorList = INSPECTORLIST;
}

// Main constructor for the passenger. 
Passenger::Passenger(int ID, List* bags, Ticket T, Airport* A){
	id = ID;
	airport = A;

	luggageList = bags;

	ticket = T;
	securityPass = true;
	airline = -1;
}
// Limited constructors for tests
Passenger::Passenger() {}
Passenger::Passenger(int ID)
{
    id = ID;
}
Passenger::Passenger(int ID, Airport* a)
{
    id = ID;
    airport = a;
}
Passenger::Passenger(int ID, Ticket t)
{
    id = ID;
    ticket = t;
}
Passenger::Passenger(int ID, Ticket t, int airlineCode, Airport* a)
{
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

Passenger::Passenger(int ID,int qIndex,Airport* AIRPORT,int airlineCode,SecurityInspector** INSPECTORLIST) {
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

Passenger::Passenger(int ID,Airport* A, BoardingPass bp) {
	id = ID;
	airport = A;
	boardingPass = bp;
}

Passenger::~Passenger(){
}

//----------------------------------------------------------------------
// This function iterates through a given array and sets the location
// of the smallest value and the value itself to the location and 
// minValue variables that are passed in
//----------------------------------------------------------------------
int Passenger::findShortestLine(List** list, bool CISline, bool Screenline){//, //int *location, int *minValue){
	int location = 0;
	int minValue = 0;				//this is the size and location of the smallest line 
	
	if(!CISline && !Screenline){
		minValue = list[0]->Size();

		for(int i = 0; i < 7; i++){
			//printf("Size: %d\n", list[i]->Size());
			if(minValue > list[i]->Size()){
				minValue = list[i]->Size();
				location = i;
			}
		}
		return location;						//Found a line
	}

	else if (CISline && !Screenline) {
        int CIS_ID = airline * 6 + 1;
		minValue = airport->passengerList->Size();
		for(int i = CIS_ID; i < CIS_ID + 5; i++){
			if(minValue > list[i]->Size() && airport->checkinState[i] != CI_CLOSED){
				minValue = list[i]->Size();
				location = i;			}
		}
		return location;						//Found a line
	}

	else {
		printf("size: %d\n", list[0]->Size());
		for(int i = 0; i < airport->screeningOfficerList->Size(); i++){
			
			if(minValue > list[i]->Size()){
				minValue = list[i]->Size();
				location = i;
			}
		}
		return location;
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
void Passenger::findShortestLiaisonLine(){
	int myLine = 0;
	airport->liaisonLineLock->Acquire();																					
		myLine = findShortestLine(airport->liaisonQueues, false, false);				// passenger will find shortest line
		
		printf("Passenger %d chose liaison %d with a line length of %d\n", 
			id, myLine, airport->liaisonQueues[myLine]->Size());
		airport->liaisonQueues[myLine]->Append((void *)this);
		if(airport->liaisonState[myLine] == L_BUSY){						// If the liaison is busy
			//Wait in line
						// add passenger to queue
			//printf("Size: %d\n", airport->liaisonQueues[myLine]->Size());
			airport->liaisonLineCV[myLine]->Wait(airport->liaisonLineLock);
		}
		else
			airport->liaisonLineLock->Release();
		
		airport->liaisonLock[myLine]->Acquire();

		//Give liaison information
		airport->liaisonCV[myLine]->Signal(airport->liaisonLock[myLine]);
		airport->liaisonLock[myLine]->Release();


	printf("Passenger %d of Airline %d is directed to the airline counter.\n", 
		id, ticket.airline);

	
	//Links to next interaction at the checkin counter
	if(airport->checkInStaffList->Size() > 0)
		CheckIn();
	
}

void Passenger::Screening() {
	int myLine = 0;
		airport->screenLineLock->Acquire();
		myLine = findShortestLine(airport->screenQueues,false, true);
		printf("Passenger %d is joining Screening officer's %d queue with length of %d\n",
		 id, myLine,airport->screenQueues[myLine]->Size());

		airport->screenQueues[myLine]->Append((void *)this);
		
		if(airport->screenState[myLine] == SO_BUSY){
			
			airport->screenlineCV[myLine]->Wait(airport->screenLineLock);
		}
		else
			airport->screenLineLock->Release();
		
		printf("asdfasfasd\n");
		airport->screenLocks[myLine]->Acquire();

		//Give bag to officer
		printf("Passenger %d gives the hand-luggage to screening officer %d\n",id, myLine);
		airport->screenCV[myLine]->Wait(airport->screenLocks[myLine]);

		if(!airport->securityInspectorList->IsEmpty())
			Inspecting();
}

void Passenger::Inspecting() {
	srand(time(NULL));
	int myLine = 0;
	airport->securityQueuesLock->Acquire();
	myLine = findShortestLine(airport->securityQueues, false,true);

	printf("Passenger %d chose security %d with a line length of %d\n", id,
			myLine, airport->securityQueues[myLine]->Size());
	airport->securityQueues[myLine]->Append((void *) this);

	if (airport->securityState[myLine] == SC_BUSY) {
		airport->securitylineCV[myLine]->Wait(airport->securityQueuesLock);
	} else
		airport->securityQueuesLock->Release();

	airport->securityLocks[myLine]->Acquire();
	if (securityPass) {
		//give security information
		airport->securitylineCV[myLine]->Signal(airport->securityLocks[myLine]);
		airport->securityLocks[myLine]->Release();
	} else {
		//yield random cycles
		int randNum = rand() % 5 + 1;
		for (int i = 0; i < randNum; i++) {
			currentThread->Yield();
		}
		airport->securityQueuesLock->Acquire();
		airport->returnQueues[myLine]->Append((void*) this);

		if (airport->securityState[myLine] == SC_BUSY) {
			airport->securitylineCV[myLine]->Wait(airport->securityQueuesLock);
		} else
			airport->securityQueuesLock->Release();

		//give security information
		airport->securitylineCV[myLine]->Signal(airport->securityLocks[myLine]);
		airport->securityLocks[myLine]->Release();
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

void Passenger::CheckIn()
{
    airport->checkinLineLock[airline]->Acquire();
    // Find the shortest line to get into. Default is executive.
	int checkInLine = airline * 6;
	if ( ! ticket.executive )
    {
		checkInLine = findShortestLine(airport->checkinQueues, true, false);;
		printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
                id, airline, checkInLine, airport->checkinQueues[checkInLine]->Size());
	}
    else
    {
        printf("Passenger %d of Airline %d is waiting in the executive class line\n",
                id, airline);
    }
    airport->checkinQueues[checkInLine]->Append((void *)this);
    airport->checkinLineCV[checkInLine]->Wait(airport->checkinLineLock[airline]);
    airport->checkinLineLock[airline]->Acquire();
    printf("Passenger %d of Airline %d was informed to board at gate %d\n",
            id, airline, boardingPass.gate);
    airport->checkinLineLock[airline]->Release();

    if(airport->screeningOfficerList->Size() > 0)
		Screening();
}
