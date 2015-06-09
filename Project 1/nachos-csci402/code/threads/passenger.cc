/*This is where all of the functions for the passenger class are defined 
*/
#include "passenger.h"

//#include "system.h"

// Main constructor for the passenger. 
Passenger::Passenger(int ID, Luggage *bags, Ticket T, int *liaisonArray){
	id = ID;
	for(int i =0; i < 7; i++)
		liaisonList[i] = liaisonArray[i];
	//luggage = bags;
	for(int i =0; i < 3; i++)
		luggage[i] = bags[i];
	
	ticket = T;
	securityPass = true;
	//boardingPass = NULL;
}

Passenger::~Passenger(){

}

/*This function iterates through a given array and sets the location
	of the smallest value and the value itself to the location and 
	minValue variables that are passed in*/
int Passenger::findShortestLine(int* list){//, //int *location, int *minValue){
	int location, minValue = 0;				//this is the size and location of the smallest line 
	minValue = list[0];
	for(int i = 0; i < 7; i++){
		if(minValue > list[i]){
			//printf("The value of the liaison line: %d\n", list[i]);
			minValue = list[i];
			location = i;
		}
	}
	//Found a line
	return location;
}


/*This is the first version of the function for the passenger to find 
	the shortest liaison line in the Airport. An array of liaison line 
	sizes is passed to the passenger and iterated to find the min length
	If the passenger has an executive class ticket, he/she should enter
	line 0, which is the executive line.*/
void Passenger::findShortestLiaisonLine(){

	int myLine = 0;										//the passeger will default to the executive line positon 
														// passenger will find shortest line
	myLine = findShortestLine(liaisonList);
	//Should be the first print statment 
	printf("Passenger %d chose liaison %d with a line length of %d\n", getID(), myLine, liaisonList[myLine]);
	//if(liaisonList[myLine] > 0)
	//	lineCV[myLine]->Wait(LineLock);
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

void Passenger::findShortestCheckinLine(int CheckinList[5]){
	int min = 0;					//this is the size 
									//of the smallest line 
	int CheckinID = 0;				//This is the id of the checkin staff
	if(this.ticket.executive){
		min = CheckinList[0];
	}	

	else {
		this.findShortestLine(CheckinList, CheckinID, min);
	}
}
*/
