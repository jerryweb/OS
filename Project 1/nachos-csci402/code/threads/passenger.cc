/*This is where all of the functions for the passenger class are defined 
*/
#include "passenger.h"

//#include "system.h"

// Main constructor for the passenger. 
Passenger::Passenger(int ID, List* bags, Ticket T, int *liaisonArray, int *checkInStaffArray){
	id = ID;
	for(int i =0; i < 7; i++)
		liaisonList[i] = liaisonArray[i];

	for(int i =0; i < 7; i++)
		checkInStaffList[i] = checkInStaffArray[i];
	luggageList = bags;

	ticket = T;
	securityPass = true;
	//boardingPass = NULL;
}

Passenger::~Passenger(){

}

/*This function iterates through a given array and sets the location
	of the smallest value and the value itself to the location and 
	minValue variables that are passed in*/
int Passenger::findShortestLine(int* list, bool CISline){//, //int *location, int *minValue){
	int location, minValue = 0;				//this is the size and location of the smallest line 
	

	if(!CISline){
		minValue = list[0];
		for(int i = 0; i < 7; i++){
			if(minValue > list[i]){
				minValue = list[i];
				location = i;
			}
		}
		return location;						//Found a line
	}

	else if(!ticket.executive && CISline){
		minValue = list[1];
		for(int i = 1; i < 5; i++){
			if(minValue > list[i]){
				minValue = list[i];
				location = i;			}
		}
		return location;						//Found a line
	}

	else
		return location;

}


/*This is the first version of the function for the passenger to find 
	the shortest liaison line in the Airport. An array of liaison line 
	sizes is passed to the passenger and iterated to find the min length
	If the passenger has an executive class ticket, he/she should enter
	line 0, which is the executive line.*/
void Passenger::findShortestLiaisonLine(){
	int myLine = 0;																					
	myLine = findShortestLine(liaisonList, false);				// passenger will find shortest line
	
	//Should be the first print statment 
	printf("Passenger %d chose liaison %d with a line length of %d\n", getID(), myLine, liaisonList[myLine]);
	printf("Passenger %d of Airline %d is directed to check-in counter\n", getID(), ticket.airline);
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
	int myLine = 0;										//the passeger will default to the executive line positon 

	if(!ticket.executive){
		myLine = findShortestLine(checkInStaffList, true);				// passenger will find shortest CIS economy line
		printf("Passenger %d of Airline %d chose Airline Check-In staff %d with a line length %d\n", 
		getID(), ticket.airline, myLine, checkInStaffList[myLine]);
	}	

	
}

