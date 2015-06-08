//This is the check-in staff class 

#ifndef CHECKIN_H
#define CHECKIN_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class CheckIn : public Thread {
	public:
		CheckIn(int airline, int id, Airport* airport);
		~CheckIn();
		void doStuff();

	private:
        int airline;        // Airline that the staff belongs to.
		int id;             // ID of the check-in staff. Only unique within airlines.
        Airport* airport;   // "Airport"" construct, containing all public data.
        int passengers;     // Records how many passengers this individual staff has processed.
        int luggage;        // Records how many bags this individual staff has processed.
        int weight;         // Records the total weight of bags this individual staff has processed.
}