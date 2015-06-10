#include "liaison.h"

Liaison::Liaison(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    passengers = new int[airport->numAirlines];
    totalLuggageWeight = new int[airport->numAirlines];
    airport->liaisonState[id] = L_BUSY;
}

Liaison::~Liaison()
{
    delete[] passengers;
    delete[] totalLuggageWeight;
}
/*
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
            airport->liaisonState[id] = L_BUSY;
        }
        else
        {   // If line is empty, do nothing; also make sure state is set correctly.
            pass = NULL;
            airport->liaisonState[id] = L_FREE;
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
        printf("Airport Liaison %d directed passenger %d of airline %d\n",
                id, pass->getID(), passAirline);
        airport->liaisonState[id] = L_FREE;
        airport->liaisonLock[id]->Release();
        // airport->liaisonCV[id]->Signal(liaisonLock[id]); ???
    }
}
*/
//----------------------------------------------------------------------
//  This Function handels the interaction between the Liaison and the Passengers
//  First the Liaison will aquire the lock and see if there are any Passengers 
//  waiting in his respective queue. The liaison will tell the passenger which 
// airline check-in counter to go to, then remove that passenger from the liaison's 
// queue
//----------------------------------------------------------------------
void Liaison::DirectPassengers(){
    Passenger* p = NULL;

    while(true){
        // Check line for passengers.

        airport->LineLock->Acquire();
        if (airport->liaisonQueues[id]->Size() > 0){
            // If line is not empty, signal next passenger.
            airport->lineCV[id]->Signal(airport->LineLock);//liaisonLock[id]);
            p = (Passenger*)airport->liaisonQueues[id]->Remove();
           
            printf("Airport Liaison %d directed passenger %d of airline %d\n", 
            id, p->getID(), p->getTicket().airline);
            airport->liaisonState[id] = L_BUSY;           
            
            // This adds the statistics for # of passengers and weight of bags for each
            // of the airlines

            
        }

        // If line is empty, do nothing; also make sure state is set correctly.
        else{
            p = NULL;
            airport->liaisonState[id] = L_FREE;
            printf("Nothing to do....\n");
        }

        airport->liaisonLock[id]->Acquire();
        airport->LineLock->Release();
        airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
        
    }
}