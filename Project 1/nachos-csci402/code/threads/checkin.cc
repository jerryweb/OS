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

void CheckIn::doStuff()
{
    while (true) // need to change this
    {
        Passenger* pass = NULL;
        if (! airport->checkinQueues[airline][0]->IsEmpty()) // executive line
        {
            pass = (Passenger*)airport->checkinQueues[airline][0]->Remove();
        }
        else if (! airport->checkinQueues[airline][id]->IsEmpty()) // economy line
        {
            pass = (Passenger*)airport->checkinQueues[airline][id]->Remove();
        }
        else currentThread->Sleep(); // ^PRETTY SURE THIS WON'T WORK PROPERLY
        int passAirline = pass->ticket.airlineCode;
        passengers++;
        for (int i = 0; i < 3; i++)
        {
            Luggage bag = pass->luggage[i];
            bag.airlineCode = passAirline;
            airport->conveyor->Append(&bag);
            luggage++;
        }
        pass->seatNumber = seatsAssigned[airline];
        seatsAssigned[airline]++;
        /*CV wait list*/->Signal();
    }
}