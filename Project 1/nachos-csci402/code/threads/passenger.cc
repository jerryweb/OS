// This is where all of the functions for the passenger class are defined 

#include "passenger.h"
#include "system.h"

// Main constructor for the passenger. 
Passenger::Passenger(int id, Luggage* luggage, Ticket ticket){
	this.id = id;
	this.luggage = luggage;
	this.ticket = ticket;
	this.securityPass = true;
}

Passenger::~Passenger(){

}

/*this function iterates through a given array and returns the location
	of the smallest value and the value itself*/
void Passenger::findShortestLine(int* list, int *location, int *minValue){
	location = 0;				//this is the size 
								//of the smallest line 
	minValue = 0;				//This is the id of the liaison
	for(int i = 0; i < list.size(); i++){
		if(minValue > list[i]){
			minValue = list.[i];
			location = i;
		}
	}
}


/*This is the first version of the function for the passenger to find 
	the shortest liaison line in the Airport. An array of liaison line 
	sizes is passed to the passenger and iterated to find the min length*/
int Passenger::findShortestLiaisonLine(int liaisonList[7]){
	int minimum = 0;				//this is the size 
									//of the smallest line 
	int liaisonID = 0;				//This is the id of the liaison
	if(this.ticket.executive){
		minimum = liaisonList[0];
	}	

	else {
		this.findShortestLine(liaisonList, liaisonID, minimum);
	}
	// printf("Passenger %d chose liaison %d with a line length of %d\n", this.id, liaisonID, minimum);

	//liaisonList[liaisonID]->append((void *)this);
	
	return liaisonID;

}

void Passenger::SetScreenPass(bool pnp) {
	this.screenPass = pnp;
}

int Passenger::GetID() {
	return this.id;
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

