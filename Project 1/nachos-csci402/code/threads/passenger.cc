/*This is where all of the functions for the passenger class are defined 
*/
#include "passenger.h"
//for random number
#include "stdlib.h"
#include "time.h"
//workaround for circular dependency
#include "securityinspector.h"

//#include "system.h"

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

Passenger::~Passenger(){
}

//----------------------------------------------------------------------
// This function iterates through a given array and sets the location
// of the smallest value and the value itself to the location and 
// minValue variables that are passed in
//----------------------------------------------------------------------
int Passenger::findShortestLine(List** list, bool CISline){//, //int *location, int *minValue){
	int location = 0;
	int minValue = 0;				//this is the size and location of the smallest line 
	
	if(!CISline){
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

	else {
        int CIS_ID = airline * 6 + 1;
		minValue = airport->passengerList->Size();
		for(int i = CIS_ID; i < CIS_ID + 5; i++){
			if(minValue > list[i]->Size() && airport->checkinState[i] != CI_CLOSED){
				minValue = list[i]->Size();
				location = i;			}
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
void Passenger::findShortestLiaisonLine(){
	int myLine = 0;
	airport->liaisonLineLock->Acquire();																					
		myLine = findShortestLine(airport->liaisonQueues, false);				// passenger will find shortest line
		
		printf("Passenger %d chose liaison %d with a line length of %d\n", 
			getID(), myLine, airport->liaisonQueues[myLine]->Size());
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
	int oldQueueIndex = queueIndex;
	//ping-pong with Screening Officer
	airport->screenLocks[queueIndex]->Acquire();
	airport->passengerWaitOfficerCV[queueIndex]->Wait(
			airport->screenLocks[queueIndex]);
	//maybe do something here, at the moment nothing
	airport->screenLocks[oldQueueIndex]->Acquire();
	airport->officerWaitPassengerCV[oldQueueIndex]->Signal(
			airport->screenLocks[oldQueueIndex]);
	airport->screenLocks[oldQueueIndex]->Release();
}

void Passenger::Inspecting() {
	airport->securityLocks[queueIndex]->Acquire();
	airport->passengerWaitInspectorCV[queueIndex]->Wait(airport->securityLocks[queueIndex]);

	if (!securityPass) {
		printf("passenger %d 172\n",id);
		airport->securityLocks[queueIndex]->Acquire();
		printf("passenger %d 174\n",id);
		airport->inspectorWaitPassengerCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
		printf("passenger %d 176\n",id);
		airport->securityLocks[queueIndex]->Release();
		printf("passenger %d 178\n",id);

		srand(time(NULL));
		int randNum = rand() % 5 + 1;
		for (int i=0;i<randNum;i++) {
			//airport->securityLocks[queueIndex]->Acquire();
			currentThread->Yield();
			//airport->securityLocks[queueIndex]->Release();
			printf("passenger yield\n");
		}

		airport->securityLocks[queueIndex]->Acquire();
		airport->lastCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
		airport->securityLocks[queueIndex]->Release();

		printf("passenger %d 182\n",id);
		airport->securityLocks[queueIndex]->Acquire();
		//inspectorList[queueIndex]->setReturn();
		//inspectorList[queueIndex]->setReturnPassenger(this);
		printf("passenger 186\n");
		airport->returnQueues[queueIndex]->Append(this);
		printf("passenger 188\n");
		airport->securityLocks[queueIndex]->Release();

		printf("passenger 191\n");
		airport->securityLocks[queueIndex]->Acquire();
		printf("passenger 193\n");
		airport->rePassengerWaitInspectorCV[queueIndex]->Wait(airport->securityLocks[queueIndex]);
		printf("passenger 195\n");
		airport->securityLocks[queueIndex]->Acquire();
		printf("passenger 197\n");
		airport->inspectorWaitRePassengerCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
		printf("passenger 199\n");
		airport->securityLocks[queueIndex]->Release();
		printf("passenger 201\n");

	} else {
		airport->securityLocks[queueIndex]->Acquire();
		airport->inspectorWaitPassengerCV[queueIndex]->Signal(airport->securityLocks[queueIndex]);
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

void Passenger::CheckIn()
{
    airport->checkinLineLock[airline]->Acquire();
    // Find the shortest line to get into. Default is executive.
	int checkInLine = airline * 6;
	if ( ! ticket.executive )
    {
		checkInLine = findShortestLine(airport->checkinQueues, true);
		printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
                id, airline, checkInLine, airport->checkinQueues[checkInLine]->Size());
	}
    else
    {
        printf("Passenger %d of Airline %d is waiting in the executive class line\n",
                id, airline);
    }
    // if( airport->checkinState[checkInLine] == CI_BUSY )
    // {   // Wait in line if check-in staff is busy.
        airport->checkinQueues[checkInLine]->Append((void *)this);
        airport->checkinLineCV[checkInLine]->Wait(airport->checkinLineLock[airline]);
    // }
    airport->checkinLineLock[airline]->Acquire();
    printf("Passenger %d of Airline %d was informed to board at gate %d\n",
            id, airline, boardingPass.gate);
    airport->checkinLineLock[airline]->Release();
}
