#include "checkin.h"

CheckIn::CheckIn(int airline, int id, Airport* airport)
{
    this.airline = airline;
	this.id = id;
    this.airport = airport;
    this.passengers = 0;
    this.luggage = 0;
}

CheckIn::~CheckIn()
{
    airport = NULL;
}

void CheckIn::doStuff() // TODO: ADD SYNCHRONIZATION.
{
    Passenger* pass = NULL;
    bool exec;
    while (true) // TODO: change this to avoid busy waiting.
    {
        // Check the lines, sleep if both empty.
        if (! airport->checkinQueues[airline][0]->IsEmpty()) // executive line
        {
            exec = true;
            pass = (Passenger*)airport->checkinQueues[airline][0]->Remove(); // TODO: get first, remove later.
            printf("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d", id, airline, airport->checkinQueues[airline][id]->Size());
        }
        else if (! airport->checkinQueues[airline][id]->IsEmpty()) // economy line
        {
            exec = false;
            pass = (Passenger*)airport->checkinQueues[airline][id]->Remove(); // TODO: get first, remove later.
            printf("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d", id, airline, airport->checkinQueues[airline][0]->Size());
        }
        else currentThread->Sleep(); // TODO: MAKE THIS CHECK HAPPEN FIRST.
        // Process ticket, create boarding pass.
        int passAirline = pass->ticket.airlineCode;
        passengers++;
        BoardingPass bp;
        bp.seatNum = airport->airlines[airline]->seatsAssigned;
        airport->airlines[airline]->seatsAssigned++;
        bp.gate = airline;
        pass->boardingPass = bp;
        if (exec) printf("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d", id, airline, pass->id, airline);
        else printf("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d", id, airline, pass->id, airline);
        // Process luggage and add it to conveyor.
        for (int i = 0; i < 3; i++)
        {
            Luggage bag = pass->luggage[i];
            bag.airlineCode = passAirline;
            airport->conveyor->Append(&bag);
            luggage++;
            weight += bag.weight;
        }
        printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system", id, airline);
        /* TODO: use CV wait list. */->Signal();
        // Check if all passengers are processed, close if done.
        if (airport->airlines[airline]->seatsAssigned >= airport->airlines[airline]->ticketsIssued)
        {
            printf("Airline check-in staff %d is closing the counter", id);
            // TODO: use currentThread->Finish() to "close"?
        }
    }
}