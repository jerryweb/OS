#include "liaison.h"

Liaison::Liaison(int id, Airport* airport)
{
	this.id = id;
    this.airport = airport;
    this.passengers = {0};
    this.luggage = {0};
}

Liaison::~Liaison()
{
    airport = NULL;
}

void Liaison::doStuff() // TODO: ADD SYNCHRONIZATION.
{
    while (true) // TODO: change this to avoid busy waiting.
    {
        // Check line for passengers.
        if (airport->liaisonQueues[id]->IsEmpty()) currentThread->Sleep();
        Passenger* pass = (Passenger*)liaisonQueues[id]->Remove(); // TODO: get first, remove later.
        // Process passenger's ticket and direct them to proper check-in line.
        int passAirline = pass->ticket.airlineCode;
        passengers[passAirline]++;
        luggage[passAirline] += 3; // TODO: change to add 2 or 3 depending on how many bags.
        pass->airlineCode = passAirline;
        printf("Airport Liaison %d directed passenger %d of airline %d",
                id, pass->id, passAirline);
        /* TODO: use CV wait list. */->Signal();
    }
}

//This isn't complete and needs a LOT more work
int  takeTicket(Ticket* ticket){


    return ticket.airline;
}