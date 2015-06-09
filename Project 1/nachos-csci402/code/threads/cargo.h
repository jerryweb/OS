// Check-in staff class. Takes luggage from the global conveyor, puts
//  it in its proper airplane, and records statistics for the manager.

#ifndef CARGO_H
#define CARGO_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

class Cargo
{
	public:
		Cargo(int id, Airport* airport);
		~Cargo();
		void Run();

	private: // TODO: change from hard-coded numbers.
		int id;             // ID of the cargo handler. Unique.
        Airport* airport;   // "Airport"" construct, containing all
                            //  public data.
        int* luggage;       // Records how many bags this individual
                            //  staff has processed, split by airline.
        int* weight;        // Records the total weight of bags this
                            //  individual staff has processed, split
                            //  by airline.
}