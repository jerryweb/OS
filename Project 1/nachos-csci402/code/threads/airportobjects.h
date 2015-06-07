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

struct BoardingPass {
	int gate;
	int seatNum;
}

class Airport {
    public:
        // numbers currently hard-coded
        Airline* airlines[3];
        List* aircraft[3];
        List* liaisonQueues[7];
        List* checkinQueues[3][5];
        List* conveyor;
        // add other lists/variables here
        //newly added Kevin
        	List* screenQueues[3];
        	List* securityQueues[3];
}

class Airline {
    public:
        // id = index in airport list
        int seatsAssigned;
        int ticketsIssued;
}

