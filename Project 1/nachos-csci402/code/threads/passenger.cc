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
// Basic constructor for tests
Passenger::Passenger(int ID)
{
    id = ID;
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
		minValue = list[1]->Size();
		for(int i = 1; i < 5; i++){
			if(minValue > list[i]->Size()){
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
	airport->LineLock->Acquire();																					
		myLine = findShortestLine(airport->liaisonQueues, false);				// passenger will find shortest line
		
		printf("Passenger %d chose liaison %d with a line length of %d\n", 
			getID(), myLine, airport->liaisonQueues[myLine]->Size());

		if(airport->liaisonState[myLine] == L_BUSY){						// If the liaison is busy
			//Wait in line
			airport->liaisonQueues[myLine]->Append((void *)this);			// add passenger to queue
			printf("Size: %d\n", airport->liaisonQueues[myLine]->Size());
			airport->lineCV[myLine]->Wait(airport->LineLock);
		}

	printf("Passenger %d of Airline %d is directed to the airline counter.\n", 
		id, ticket.airline);
	airport->LineLock->Release();

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


void Passenger::findShortestCheckinLine(){
	/*int myLine = 0;										//the passeger will default to the executive line positon 

	if(!ticket.executive){
		myLine = findShortestLine(airport->checkinQueues, true);				// passenger will find shortest CIS economy line
		printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
		getID(), ticket.airline, myLine, checkInStaffList[myLine]);
	}	
	*/
	
}

