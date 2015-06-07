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

struct BoardingPass {
	int gate;
	int seatNum;
}

class Airport {
    public:
        List* liaisonQueues[7];
        List* checkinQueues[3][5];
        List* conveyor;
        int seatsAssigned[3];
        // add other lists/variables here
}