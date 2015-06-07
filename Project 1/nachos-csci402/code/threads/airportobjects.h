#ifndef AIRPORTOBJECTS_H
#define AIRPORTOBJECTS_H

#include "copyright.h"
#include "thread.h"
#include "list.h"


struct Luggage {
	int airlineCode;
	int weight;
}

struct Ticket {
	int airline;
	bool executive;
}

	
	List* liasionQueues[7];