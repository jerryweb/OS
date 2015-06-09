#include "liaison.h"

Liaison::Liaison(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    passengers = {0};
    luggage = {0};
}

Liaison::~Liaison()
{
    airport = NULL;
}

void Liaison::Run()
{
    Passenger* pass = NULL;
    while (true)
    {
        // Check line for passengers.
        airport->liaisonLineLock->Acquire();
        if (airport->liaisonQueues[id]->Size() > 0)
        {   // if line is not empty:
            airport->liaisonCV[id]->Signal(/*lock*/);
            pass = (Passenger*)airport->liaisonQueues[id]->Remove();
            airport->liaisonState[id] = BUSY;
        }
        else
        {   // if line is empty:
            pass = NULL;
            airport->liaisonState[id] = FREE;
        }
        airport->liaisonLock[id]->Acquire();
        airport->liaisonLineLock->Release();
        airport->liaisonCV[id]->Wait(/*lock*/);
        
        /*
        
        if (airport->liaisonQueues[id]->IsEmpty()) currentThread->Sleep();
        
        // Process passenger's ticket and direct them to proper check-in line.
        int passAirline = pass->ticket.airlineCode;
        passengers[passAirline]++;
        luggage[passAirline] += 3; // TODO: change to add 2 or 3 depending on how many bags.
        pass->airlineCode = passAirline;
        printf("Airport Liaison %d directed passenger %d of airline %d",
                id, pass->id, passAirline);
        cv->Signal();
        
        */
    }
}
