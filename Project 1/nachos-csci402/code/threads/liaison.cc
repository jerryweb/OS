#include "liaison.h"

Liaison::Liaison(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    passengers = new int[airport->numAirlines];
    luggage = new int[airport->numAirlines];
}

Liaison::~Liaison()
{
    delete passengers;
    delete luggage;
}

void Liaison::Run()
{
    Passenger* pass = NULL;
    while (true)
    {   // TODO: WHICH LOCK SHOULD liaisonCV USE?
        // Check line for passengers.
        airport->liaisonLineLock->Acquire();
        if (airport->liaisonQueues[id]->Size() > 0)
        {   // If line is not empty, signal next passenger.
            airport->liaisonCV[id]->Signal(liaisonLock[id]);
            pass = (Passenger*)airport->liaisonQueues[id]->Remove();
            airport->liaisonState[id] = BUSY;
        }
        else
        {   // If line is empty, do nothing.
            pass = NULL;
            airport->liaisonState[id] = FREE;
        }
        airport->liaisonLock[id]->Acquire();
        airport->liaisonLineLock->Release();
        airport->liaisonCV[id]->Wait(liaisonLock[id]);
        
        // TODO: figure out what happens below...
        
        // passenger signals when it's "given ticket to liaison"
        
        // Process passenger's ticket and direct them to proper check-in line.
        int passAirline = pass->getTicket().airlineCode;
        Luggage* passLuggage = pass->getLuggage();
        passengers[passAirline]++;
        if (passLuggage[3] == NULL) luggage[passAirline] += 2;
        else                        luggage[passAirline] += 3;
        pass->setAirlineCode(passAirline);
        printf("Airport Liaison %d directed passenger %d of airline %d",
                id, pass->getID(), passAirline);
        airport->liaisonState[id] = FREE;
        airport->liaisonLock[id]->Release();
        // airport->liaisonCV[id]->Signal(liaisonLock[id]); ???
    }
}
