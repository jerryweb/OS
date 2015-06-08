//This is the liaison class 

#ifndef LIAISON_H
#define LIAISON_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Liaison : public Thread {
	public:
		Liaison(int id, Airport* airport);
		~Liaison();
		void doStuff();

		void takeTicket(Ticket* ticket);


	private: // TODO: change from hard-coded numbers.
		int id;             // ID of the liaison. Unique.
        Airport* airport;   // "Airport"" construct, containing all public data.
        int passengers[3];  // Records how many passengers this individual staff has processed, split by airline.
        int luggage[3];     // Records how many bags this individual staff has processed, split by airline.
}