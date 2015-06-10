/*This is where all of the functions for the passenger class are defined 
*/
#include "passenger.h"

//#include "system.h"

// Main constructor for the passenger. 
Passenger::Passenger(int ID, List* bags, Ticket T, Airport* A, List** newLiaisonList, 
	 int *checkInStaffArray){
	id = ID;
	airport = A;
	
	for(int i =0; i < 7; i++){
		//liaisonList[i] = liaisonArray[i];
		liaisons[i] = newLiaisonList[i];
		// int testV = 9;
		// int testV2 = 10;
		// liaisons[i]->Append((void *)testV);
		// liaisons[i]->Append((void *)testV2);
		//printf("This is the liaison queue %d\n", liaisons[i]->First());
	}

	for(int i =0; i < 5; i++)
		checkInStaffList[i] = checkInStaffArray[i];

	luggageList = bags;

	ticket = T;
	securityPass = true;
	//boardingPass = NULL;
}
// Limited constructors for tests
Passenger::Passenger() {}
Passenger::Passenger(int ID)
{
    id = ID;
}
Passenger::Passenger(int ID, Ticket t, int airlineCode, Airport* a)
{
    id = ID;
    ticket = t;
    airline = airlineCode;
    airport = a;
}

Passenger::~Passenger(){
}

//----------------------------------------------------------------------
// This function iterates through a given array and sets the location
// of the smallest value and the value itself to the location and 
// minValue variables that are passed in
//----------------------------------------------------------------------
int Passenger::findShortestLine(List** list, bool CISline){//, //int *location, int *minValue){
	int location, minValue = 0;				//this is the size and location of the smallest line 
	

	if(!CISline){
		minValue = list[0]->Size();
		for(int i = 0; i < 7; i++){
			if(minValue > list[i]->Size()){
				//printf("NO\n");
				minValue = list[i]->Size();
				location = i;
			}
		}
		return location;						//Found a line
	}

	else if(!ticket.executive && CISline){
        int CIS_ID = airline * 6 + 1;
		minValue = airport->passengerList->Size();
		for(int i = CIS_ID; i < CIS_ID + 5; i++){
			if(minValue > list[i]->Size() && airport->checkinState[i] != CI_BREAK){
				minValue = list[i]->Size();
				location = i;			}
		}
		return location;						//Found a line
	}

	else
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
void Passenger::findShortestLiaisonLine(){
	int myLine = 0;
	airport->liaisonLineLock->Acquire();																					
		myLine = findShortestLine(airport->liaisonQueues, false);				// passenger will find shortest line
		
		printf("Passenger %d chose liaison %d with a line length of %d\n", 
			getID(), myLine, airport->liaisonQueues[myLine]->Size());

		if(airport->liaisonState[myLine] == L_BUSY){						// If the liaison is busy
			//Wait in line
			airport->liaisonQueues[myLine]->Append((void *)this);			// add passenger to queue
			//printf("Size: %d\n", airport->liaisonQueues[myLine]->Size());
			airport->liaisonLineCV[myLine]->Wait(airport->liaisonLineLock);
		}

	printf("Passenger %d of Airline %d is directed to the airline counter.\n", 
		id, ticket.airline);
	airport->liaisonLineLock->Release();

}
/*
void Passenger::SetSecurityPass(bool pnp) {
	this.screenPass = pnp;
}

bool Passenger::GetSecurityPass() {
	return this.SecurityPass;
}

void Passenger::Questioning() {

}
*/

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
    if( airport->checkinState[checkInLine] == CI_BUSY )
    {   // Wait in line if check-in staff is busy.
        airport->checkinQueues[checkInLine]->Append((void *)this);
        airport->checkinLineCV[checkInLine]->Wait(airport->checkinLineLock[airline]);
    }
    printf("Passenger %d of Airline %d was informed to board at gate %d\n",
            id, airline, boardingPass.gate);
    airport->checkinLineLock[airline]->Release();
}
