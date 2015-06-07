#ifndef AIRPORTOBJECTS_H
#define AIRPORTOBJECTS_H

#include "copyright.h"
#include "thread.h"
#include "list.h"


struct Luggage {
	int airlineCode;		//code for the distinction of airlines 
	int weight;				//weight of bags in lbs
}

struct Ticket {
	int airline;
	bool executive;			//if true, then passenger is in executive class, otherwise passenger is 
							//economy class
}

	
	//List* liasionQueues[7];