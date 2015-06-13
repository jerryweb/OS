#include "liaison.h"

Liaison::Liaison(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    passengers = new int[airport->numAirlines];
    luggageCount = new int[airport->numAirlines];
    totalLuggageWeight = new int[airport->numAirlines];     //May not be needed
    for(int i= 0; i < airport->numAirlines; i++){
        passengers[i] = 0;
        luggageCount[i] = 0;
    }
    airport->liaisonState[id] = L_BUSY;
}

Liaison::~Liaison()
{
    delete[] passengers;
    delete[] totalLuggageWeight;
}

// Finds the first passenger in the list (if applicable).
Passenger* Liaison::CheckForPassengers()
{
    Passenger* p;
    if (airport->liaisonQueues[id]->Size() > 0)
    {   // If line is not empty, signal next passenger.
        airport->liaisonLineCV[id]->Signal(airport->liaisonLineLock);
        p = (Passenger*)airport->liaisonQueues[id]->Remove();
       
        printf("Airport Liaison %d directed passenger %d of airline %d\n", 
                id, p->getID(), p->getTicket().airline);
        airport->liaisonState[id] = L_BUSY;           
        p->SetAirline(p->getTicket().airline);
    }
    else
    {   // If line is empty, do nothing; also make sure state is set correctly.
        p = NULL;
        airport->liaisonState[id] = L_FREE;
    }
    return p;
}

//----------------------------------------------------------------------
//  This Function handles the interaction between the Liaison and the Passengers
//  First the Liaison will acquire the lock and see if there are any Passengers 
//  waiting in his respective queue. The liaison will tell the passenger which 
// airline check-in counter to go to, then remove that passenger from the liaison's 
// queue
//----------------------------------------------------------------------
void Liaison::DirectPassengers(){
    Passenger* p = NULL;
    
    while(true){
        // Check line for passengers.
        airport->liaisonLineLock->Acquire();
        p = CheckForPassengers();

        airport->liaisonLock[id]->Acquire();
        airport->liaisonLineLock->Release();
        if(p != NULL){
            airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
            //Wait for passenger to give liaison information
            airport->liaisonLock[id]->Acquire();

            // This adds the statistics for # of passengers and weight of bags for each
            // of the airlines
        
            passengers[p->getTicket().airline]++;
           
            List *bags = p->getLuggage();                       //Temp list for iterating through luggage
            for(int j = bags->Size(); j > 0; j--){              //This calculates the weights of each of the bags 
                                                                //and puts it into a temp array to be read
                luggageCount[p->getTicket().airline]++;
            }
            airport->liaisonLock[id]->Release();
        }

        else{
                airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
         
        } 
        if(airport->RequestingLiaisonData[id]){     //prevent race conditions with other liaisons
               
                airport->liaisonManagerLock->Acquire();
               
                //Give manager data
                
                airport->liaisonLock[id]->Acquire();
                airport->liaisonManagerCV->Signal(airport->liaisonManagerLock);
                airport->liaisonManagerLock->Release();
                airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
                // //Wait for manager to signal that all the data has been collected
                airport->RequestingLiaisonData[id] = false;
        }
        
    }
}
