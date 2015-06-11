// Check-in staff class. Takes passenger's ticket and luggage, gives
//  customer a boarding pass, puts luggage on a global conveyor, and
//  records statistics for the manager.

#ifndef CHECKIN_H
#define CHECKIN_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "passenger.h"
#include "airport.h"

class CheckIn
{
	public:
		CheckIn(int airline, int id, Airport* airport);
		~CheckIn();
        Passenger* FindPassenger(int execLine);
		// void Run();
        void StartCheckInStaff();

	private:
        int airline;        // Airline that the staff belongs to.
		int id;             // ID of the check-in staff. Only unique
                            //  within airlines.
        Airport* airport;   // "Airport"" construct, containing all
                            //  public data.
        int passengers;     // Records how many passengers this
                            //  individual staff has processed.
        int luggage;        // Records how many bags this individual
                            //  staff has processed.
        int weight;         // Records the total weight of bags this
                            //  individual staff has processed.
};

#endif
