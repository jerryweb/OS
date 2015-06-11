#include "manager.h"

Manager::Manager(Airport* airport_){
    airport = airport_;
    airlineLuggage = new List*[airport->numAirlines];
	liaisonBaggageCount = new int[airport->numAirlines];    
	cargoHandlersBaggageWeight = new int[airport->numAirlines];
	cargoHandlersBaggageCount = new int[airport->numAirlines];
	checkinBaggageWeight = new int[airport->numAirlines];
    liaisonPassengerCount = new int[airport->numAirlines];	
    //Prevent Garbage values
    for(int i = 0; i < airport->numAirlines; i++){
    	liaisonPassengerCount[i] = 0;
    	liaisonBaggageCount[i] = 0;
    	cargoHandlersBaggageWeight[i] = 0;
    	cargoHandlersBaggageCount[i] = 0;
    }
    checkinPassengerCount = new List*[airport->numAirlines];
    securityInspectorPassengerCount = new List*[airport->numAirlines];
}

Manager::~Manager(){
    delete[] airlineLuggage;
    delete[] liaisonBaggageCount;
    delete[] cargoHandlersBaggageWeight;
    delete[] cargoHandlersBaggageCount;
    delete[] checkinBaggageWeight;
    delete[] liaisonPassengerCount;
    delete[] checkinPassengerCount;
    delete[] securityInspectorPassengerCount;
}

void Manager::MakeRounds(){
	bool cargoHandlersOnBreak = false;
	bool liaisonDone = false;
	bool CargoDone = false;
    int counter = 0;
    Liaison* L = NULL;
    Cargo* CH = NULL;
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

    	if(!liaisonDone){									//Delete this at the end of the project
    		LiaisonDataRequest(L);
    		liaisonDone = true;
    	}
    	if(!CargoDone){
    		CargoRequest(CH);
    		CargoDone = true;
    	}
    	
    	
    	for(int i = 0; i < 10; i++) 		//this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();
		
    }
}

void Manager::LiaisonDataRequest(Liaison *L){

    //Gather data from liaisons 

	for(int j = 0; j < airport->liaisonList->Size(); j++){

		airport->liaisonManagerLock->Acquire();
		
		L = (Liaison*)airport->liaisonList->Remove();
		printf("Getting data from Liaison %d\n", L->getID());
		airport->liaisonList->Append((void *)L);
		airport->RequestingLiaisonData[L->getID()] = true;

		airport->liaisonCV[L->getID()]->Signal(airport->liaisonLock[L->getID()]);
		airport->liaisonManagerCV->Wait(airport->liaisonManagerLock);
		//Waits for the signal of corresponding Liaison
		airport->liaisonLock[L->getID()]->Acquire();
		//Records the number of passengers per airline and stores into an array
		for(int k = 0; k < airport->numAirlines; k++){
			// printf("num of pass per airline: %d\n", L->getPassengers(k));
			liaisonPassengerCount[k] += L->getPassengers(k);
			liaisonBaggageCount[k] += L->getLuggageCount(k);
			// printf("Count for airline %d: %d\n", k, liaisonPassengerCount[k]);
			// printf("Baggage count for airline %d: %d\n", k, liaisonBaggageCount[k]);
		}

		//Signals liaison that all the data has been collected
		airport->liaisonCV[L->getID()]->Signal(airport->liaisonLock[L->getID()]);
		airport->liaisonLock[L->getID()]->Release();
	}
}

void Manager::CargoRequest(Cargo *CH){
	for(int i = 0; i < airport->cargoHandlerList->Size(); i++){

		airport->CargoHandlerManagerLock->Acquire();
		CH = (Cargo*)airport->cargoHandlerList->Remove();
		printf("Getting data from Cargo Handler %d\n", CH->getID());
		airport->cargoHandlerList->Append((void *)CH);
		airport->RequestingCargoData[CH->getID()] = true;
		airport->cargoCV->Broadcast(airport->cargoLock);

		printf("i is: %d\n", i);
		airport->cargoManagerCV[CH->getID()]->Wait(airport->CargoHandlerManagerLock);
		printf("i is: %d\n", i);
		//Waits for the signal of corresponding Liaison
		airport->cargoDataLock[CH->getID()]->Acquire();
		//Records the total weight per airline and stores into an array
		for(int k = 0; k < airport->numAirlines; k++){
			cargoHandlersBaggageWeight[k] += CH->getWeight(k);
			cargoHandlersBaggageCount[k] += CH->getLuggage(k);
			printf("Baggage weight for airline %d: %d\n", k, cargoHandlersBaggageWeight[k]);
		}

		//Signals liaison that all the data has been collected
		airport->cargoDataCV[CH->getID()]->Signal(airport->cargoDataLock[CH->getID()]);
		airport->cargoDataLock[CH->getID()]->Release();

	}
}