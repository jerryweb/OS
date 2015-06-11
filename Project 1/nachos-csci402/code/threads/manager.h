// 	This is the Manager class that will hold all of the major functions associated with
//	the airport manager.

#ifndef MANAGER_H
#define MANAGER_H

#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "airport.h"
#include "liaison.h"
#include "cargo.h"
#include "checkin.h"


class Manager {
	public:
		Manager(Airport* airport);
		~Manager();
		void MakeRounds();
		void LiaisonDataRequest(Liaison *L);
		void CheckinDataReuqest(CheckIn *C);
		void CargoRequest(Cargo *CH);
	private:
        Airport* airport;
        //Baggage Data   									// "Airport" construct, containing all public data.
        List** airlineLuggage;				//Holds the list of bags for each airline
        int* liaisonBaggageCount;			//For the weights just calculate in print function
		int* cargoHandlersBaggageWeight;
		int* checkinBaggageWeight;			
		int* cargoHandlersBaggageCount;
		//Passenger Data
        int* liaisonPassengerCount;	
        int* checkinPassengerCount;
        int* securityInspectorPassengerCount;
};

#endif
