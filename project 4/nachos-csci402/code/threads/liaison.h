// Liaison class. Directs passengers to a check-in area based on
//  their ticket and records statistics for the manager.

#ifndef LIAISON_H
#define LIAISON_H


#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"
#include "passenger.h"


class Liaison
{
	public:
		Liaison(int ID, Airport* airport);
		~Liaison();
        void DirectPassengers();
        int getID() { return id; }
        Passenger* CheckForPassengers();
        int getPassengers(int airline) { return passengers[airline]; }
        int getLuggageCount(int airline) { return luggageCount[airline]; }
        int getLuggageWeight(int airline) { return totalLuggageWeight[airline]; }

	private:
		int id;                     // ID of the liaison. Unique.
        Airport* airport;           // "Airport" construct, containing
                                    //  all public data.
        int* passengers;            // Records how many passengers this
                                    //  individual staff has processed,
                                    //  split by airline.
        int* luggageCount;          // Records number of bags per airline
        int* totalLuggageWeight;    // Records how many bags this
                                    //  individual staff has processed,
                                    //  split by airline.
        LiaisonState state;         // State of the liaison (FREE or BUSY).
};

#endif
