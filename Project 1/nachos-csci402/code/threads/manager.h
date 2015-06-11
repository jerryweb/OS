// 	This is the Manager class that will hold all of the major functions associated with
//	the airport manager.

#ifndef MANAGER_H
#define MANAGER_H

#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "airport.h"


class Manager {
	public:
		Manager(Airport* airport);
		~Manager();
		void MakeRounds();

	private:
        Airport* airport;
        //Baggage Data   									// "Airport" construct, containing all public data.
        List** airlineLuggage;				//Holds the list of bags for each airline
        List** liaisonBaggageCount;	//For the weights just calculate in print function
		List** cargoHandlersBaggageCount;
		List** checkinBaggageWeight;

		//Passenger Data
        List** liaisonPassengerCount;	
        List** checkinPassengerCount;
        List** securityInspectorPassengerCount;
};

#endif
