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

        airport->liaisonLineLock->Acquire();
        if (airport->liaisonQueues[id]->Size() > 0){
            // If line is not empty, signal next passenger.
            airport->liaisonLineCV[id]->Signal(airport->liaisonLineLock);//liaisonLock[id]);
            p = (Passenger*)airport->liaisonQueues[id]->Remove();
           
            printf("Airport Liaison %d directed passenger %d of airline %d\n", 
            id, p->getID(), p->getTicket().airline);
            airport->liaisonState[id] = L_BUSY;           
            p->SetAirline(p->getTicket().airline);
            // This adds the statistics for # of passengers and weight of bags for each
            // of the airlines
        }

        // If line is empty, do nothing; also make sure state is set correctly.
        else{
            p = NULL;
            airport->liaisonState[id] = L_FREE;
            //printf("Nothing to do....\n");
        }

        airport->liaisonLock[id]->Acquire();
        airport->liaisonLineLock->Release();
        airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
        
    }
}
