#include "passenger.h"

Passenger::Passenger(int id, Luggage* luggage, Ticket ticket)
{
	this.id = id;
	this.luggage = luggage;
	this.ticket = ticket;
	this.securityPass = true;
}

Passenger::~Passenger(){

}

/*This is the first version of the function for the passenger to find 
	the shortest liaison line in the Airport. An array of liaison line 
	sizes is passed to the passenger and iterated to find the min length*/
void Passenger::doStuff(int* liaisonList[7])
{
	int minimum = 0;				//this is the size 
									//of the smallest line 
	int liaisonID = 0;				//This is the id of the liaison
	for(int i = 0; i < liaisonList.size(); i++){
		if(minimum > liaisonList[i]){
			minimum = liaisonList.[i];
		}
	}

	printf("Passenger %d chose liaison %d with a line length of %d\n", this.id, liaisonID, minimum);
}

void Passenger::SetScreenPass(bool pnp) {
	this.screenPass = pnp;
}

int Passenger::GetID() {
	return this.id;
}
