#include "manager.h"

Manager::Manager(Airport* airport_)
{
    airport = airport_;
    airlineLuggage = new List*[airport->numAirlines];
	liaisonBaggageCount = new int[airport->numAirlines];    
	cargoHandlersBaggageCount = new List*[airport->numAirlines];
	checkinBaggageWeight = new int[airport->numAirlines];
    liaisonPassengerCount = new int[airport->numAirlines];	
    for(int i = 0; i < airport->numAirlines; i++){
    	liaisonPassengerCount[i] = 0;
    	liaisonBaggageCount[i] = 0;
    }
    checkinPassengerCount = new List*[airport->numAirlines];
    securityInspectorPassengerCount = new List*[airport->numAirlines];
}

Manager::~Manager()
{
    delete[] airlineLuggage;
    delete[] liaisonBaggageCount;
    delete[] cargoHandlersBaggageCount;
    delete[] checkinBaggageWeight;
    delete[] liaisonPassengerCount;
    delete[] checkinPassengerCount;
    delete[] securityInspectorPassengerCount;
}

void Manager::MakeRounds()
{
	bool cargoHandlersOnBreak = false;
	bool done = false;
    int counter = 0;
    Liaison* L = NULL;
    while(true){
    	//keeps track of how many cargo handlers are on break

    	//Cargo handler interaction

    	airport->conveyorLock->Acquire();
       	if(!airport->conveyor->IsEmpty()){

    		counter = 0;
    		for(int i = 0; i < airport->cargoHandlerList->Size(); i++){
    			if(airport->cargoState[i] == C_BREAK){
    				counter++;
    			}
    		}
    		//If all handlers are on break, broadcast 
    		if(counter == airport->cargoHandlerList->Size()){
    			printf("Waking up the handlers\n");
    			airport->cargoLock->Acquire();
    			airport->cargoCV->Broadcast(airport->cargoLock);
    			airport->cargoLock->Release();
    		}
    	}
    	airport->conveyorLock->Release();

    	//if(!done){
    	//Gather data from liaisons 
    	for(int j = 0; j < airport->liaisonList->Size(); j++){
    		airport->liaisonManagerLock->Acquire();
    		printf("J is: %d\n", j);
    		L = (Liaison*)airport->liaisonList->First();
    		 printf("Getting data from Liaison %d\n", L->getID());
    		airport->liaisonList->Remove();
    		airport->liaisonList->Append((void *)L);
    		airport->liaisonManagerCV[L->getID()]->Wait(airport->liaisonManagerLock);

    		//Waits for the signal of corresponding Liaison
    		airport->liaisonDataLock[L->getID()]->Acquire();
    		//Records the number of passengers per airline and stores into an array
    		for(int k = 0; k < airport->numAirlines; k++){
    			// printf("num of pass per airline: %d\n", L->getPassengers(k));
    			liaisonPassengerCount[k] += L->getPassengers(k);
    			liaisonBaggageCount[k] += L->getLuggageCount(k);
    			// printf("Count for airline %d: %d\n", k, liaisonPassengerCount[k]);
    			// printf("Baggage count for airline %d: %d\n", k, liaisonBaggageCount[k]);
    		}

    		//Signals liaison that all the data has been collected
    		airport->liaisonCV[L->getID()]->Signal(airport->liaisonDataLock[L->getID()]);
    		airport->liaisonDataLock[L->getID()]->Release();
    		printf("done with loop\n");
    	}
  
    	//}
    	for(int i = 0; i < 1000; i++) 		//this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();
		
    }
}
