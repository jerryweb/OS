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

void Liaison::doStuff()
{
    while (true) // need to change this
    {
        if (airport->liaisonQueues[id]->IsEmpty()) currentThread->Sleep();
        Passenger* pass = (Passenger*)liaisonQueues[id]->Remove();
        int passAirline = pass->ticket.airlineCode;
        passengers[passAirline]++;
        luggage[passAirline] += 3; // add 2 or 3 depending on how many bags
        pass->airlineCode = passAirline;
        printf("Airport Liaison %d directed passenger %d of airline %d",
                id, pass->id, passAirline);
        /*CV wait list*/->Signal();
    }
}