#include "manager.h"

Manager::Manager(Airport* airport_)
{
    airport = airport_;
    airlineLuggage = new List*[airport->numAirlines];
	liaisonBaggageCount = new List*[airport->numAirlines];    
	cargoHandlersBaggageCount = new List*[airport->numAirlines];
	checkinBaggageWeight = new List*[airport->numAirlines];
    liaisonPassengerCount = new List*[airport->numAirlines];
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
    int counter = 0;
    while(true){
    						//keeps track of how many cargo handlers are on break
    	//Cargo handler interaction
    	
    	airport->conveyorLock->Acquire();
        
        Luggage* bag = new Luggage;
        bag->airlineCode = 0;
        bag->weight = 30;
        airport->conveyor->Append((void*)bag);
        
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
    	for(int i = 0; i < 100; i++) 		//this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();
		
    }
}
