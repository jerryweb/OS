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
    airport->RequestingLiaisonData[id] = false;
    airport->liaisonState[id] = L_BUSY;
}

Liaison::~Liaison()
{
    delete[] passengers;
    delete[] totalLuggageWeight;
}

/*
-void Liaison::Run()
-{
-    Passenger* pass = NULL;
-    while (true)
-    {   // TODO: WHICH LOCK SHOULD liaisonCV USE?
-        // Check line for passengers.
-        airport->liaisonLineLock->Acquire();
-        if (airport->liaisonQueues[id]->Size() > 0)
-        {   // If line is not empty, signal next passenger.
-            airport->liaisonCV[id]->Signal(liaisonLock[id]);
-            pass = (Passenger*)airport->liaisonQueues[id]->Remove();
-            airport->liaisonState[id] = L_BUSY;
-        }
-        else
-        {   // If line is empty, do nothing; also make sure state is set correctly.
-            pass = NULL;
-            airport->liaisonState[id] = L_FREE;
-        }
-        airport->liaisonLock[id]->Acquire();
-        airport->liaisonLineLock->Release();
-        airport->liaisonCV[id]->Wait(liaisonLock[id]);
-        
-        // TODO: figure out what happens below...
-        
-        // passenger signals when it's "given ticket to liaison"
-        
-        // Process passenger's ticket and direct them to proper check-in line.
-        int passAirline = pass->getTicket().airlineCode;
-        Luggage* passLuggage = pass->getLuggage();
-        passengers[passAirline]++;
-        if (passLuggage[3] == NULL) luggage[passAirline] += 2;
-        else                        luggage[passAirline] += 3;
-        pass->setAirlineCode(passAirline);
-        printf("Airport Liaison %d directed passenger %d of airline %d\n",
-                id, pass->getID(), passAirline);
-        airport->liaisonState[id] = L_FREE;
-        airport->liaisonLock[id]->Release();
-        // airport->liaisonCV[id]->Signal(liaisonLock[id]); ???
-    }
-}
-*/

// Finds the first passenger in the list (if applicable).
Passenger* Liaison::CheckForPassengers()
{
    Passenger* p;
    if (airport->liaisonQueues[id]->Size() > 0)
    {   // If line is not empty, signal next passenger.
        airport->liaisonLineCV[id]->Signal(airport->liaisonLineLock);//liaisonLock[id]);
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
        p = CheckForPassengers();

        airport->liaisonLock[id]->Acquire();
        airport->liaisonLineLock->Release();
        airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
        //Wait for passenger to give liaison information
        airport->liaisonLock[id]->Acquire();

        // This adds the statistics for # of passengers and weight of bags for each
        // of the airlines
        if(p != NULL){
            passengers[p->getTicket().airline]++;
           
            List *bags = p->getLuggage();                       //Temp list for iterating through luggage
            for(int j = bags->Size(); j > 0; j--){              //This calculates the weights of each of the bags 
                // Luggage *l = (Luggage*)bags->First();           //and puts it into a temp array to be read
                // totalLuggageWeight[p->getTicket().airline] += l->weight;
                // bags->Remove();
                // // printf("Total weigth %d\n", totalLuggageWeight[p->getTicket().airline]);
                // bags->Append((void *)l);                 //Prevent destruction of local bags list         
                luggageCount[p->getTicket().airline]++;
            }
        }
        airport->liaisonLock[id]->Release();

    //Interaction With Manager
        //Recieve from Manager
        if(airport->RequestingLiaisonData[id]){     //prevent race conditions with other liaisons
            airport->liaisonManagerLock->Acquire();
            printf("liaison %d is sending data.\n", id);
           
            //Give manager data
            airport->liaisonManagerCV->Signal(airport->liaisonManagerLock);
            airport->liaisonLock[id]->Acquire();
            airport->liaisonManagerLock->Release();
            airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
            
            //Wait for manager to signal that all the data has been collected
            airport->liaisonLock[id]->Acquire();
            printf("liaison %d has finished reporting data to manager.\n", id);
            airport->liaisonLock[id]->Release();
            airport->RequestingLiaisonData[id] = false;
         }
    }
}
