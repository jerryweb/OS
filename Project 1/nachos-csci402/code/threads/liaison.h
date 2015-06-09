// Liaison class. Directs passengers to a check-in area based on
//  their ticket and records statistics for the manager.

#ifndef LIAISON_H
#define LIAISON_H
#endif

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

class Liaison
{
	public:
		Liaison(int id, Airport* airport);
		~Liaison();
		void Run();

	private: // TODO: change from hard-coded numbers.
		int id;             // ID of the liaison. Unique.
        Airport* airport;   // "Airport" construct, containing all
                            //  public data.
        int* passengers;    // Records how many passengers this individual
                            //  staff has processed, split by airline.
        int* luggage;       // Records how many bags this individual staff
                            //  has processed, split by airline.
        LiaisonState state; // State of the liaison (FREE or BUSY).
};