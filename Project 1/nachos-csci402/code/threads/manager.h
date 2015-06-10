// 	This is the Manager class that will hold all of the major functions associated with
//	the airport manager.

#ifndef MANAGER_H
#define MANAGER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

class Manager {
	public:
		Manager(Airport* airport);
		~Manager();
		void Run();

	private:
        Airport* airport;   		// "Airport" construct, containing all public data.
        List* airlineLuggage[airport->numAirlines];	//Holds the list of bags for each airline

};

#endif
