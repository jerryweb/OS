#include "manager.h"

Manager::Manager(Airport* airport_){
    airport = airport_;
    airlineLuggage = new List*[airport->numAirlines];
	liaisonBaggageCount = new int[airport->numAirlines];    
	cargoHandlersBaggageWeight = new int[airport->numAirlines];
	cargoHandlersBaggageCount = new int[airport->numAirlines];
	checkinBaggageWeight = new int[airport->numAirlines];
    liaisonPassengerCount = new int[airport->numAirlines];	
    checkinPassengerCount = new int[airport->numAirlines];
    securityInspectorPassengerCount = new int[airport->numAirlines];
    //Prevent Garbage values
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
	bool CheckinDone = false;
	bool ready = true;
    int counter = 0;
    Liaison* L = NULL;
    Cargo* CH = NULL;
    CheckIn* C = NULL;
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
    			printf("Airport manager calls back all the cargo handlers from break\n");
    			
    			for(int i = 0; i < airport->cargoHandlerList->Size(); i++){
    				airport->cargoLock[i]->Acquire();
    				airport->cargoDataCV[i]->Signal(airport->cargoLock[i]);
    				
    			}
    			for(int i = 0; i < airport->cargoHandlerList->Size(); i++){
    				airport->cargoLock[i]->Release();
    			}
    			
    		}
    	}
    	airport->conveyorLock->Release();

    	//These functions gather data from each of the thread classes
    	LiaisonDataRequest(L);

    	CargoRequest(CH);

		CheckinDataReuqest(C);

	    	
	    	
    	for(int i = 0; i < 50; i++) 		//this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();
		
    }
}

void Manager::LiaisonDataRequest(Liaison *L){

    //Gather data from liaisons 
	for(int i = 0; i < airport->numAirlines; i++){			//prevents multicounting 
    	liaisonPassengerCount[i] = 0;
    	liaisonBaggageCount[i] = 0;
    }
	for(int j = 0; j < airport->liaisonList->Size(); j++){
		L = (Liaison*)airport->liaisonList->Remove();
		
		airport->liaisonList->Append((void *)L);
		
		if(airport->liaisonState[j] == L_FREE){
			airport->liaisonManagerLock->Acquire();
			
			L = (Liaison*)airport->liaisonList->Remove();
			
			airport->liaisonList->Append((void *)L);
			airport->RequestingLiaisonData[L->getID()] = true;
			airport->liaisonCV[L->getID()]->Signal(airport->liaisonLock[L->getID()]);
			
			airport->liaisonManagerCV->Wait(airport->liaisonManagerLock);
			//Waits for the signal of corresponding Liaison
			airport->liaisonLock[L->getID()]->Acquire();

			//Records the number of passengers per airline and stores into an array
			for(int k = 0; k < airport->numAirlines; k++){
				liaisonPassengerCount[k] += L->getPassengers(k);
				liaisonBaggageCount[k] += L->getLuggageCount(k);
			}

			//Signals liaison that all the data has been collected
			airport->liaisonCV[L->getID()]->Signal(airport->liaisonLock[L->getID()]);
			airport->liaisonLock[L->getID()]->Release();
		}
	}
}

void Manager::CheckinDataReuqest(CheckIn *C){
	//Gather data from liaisons 
	for(int i = 0; i < airport->numAirlines; i++){			//prevents multicounting 
    	checkinPassengerCount[i] = 0;
    	checkinBaggageWeight[i] = 0;
    }

	for(int j = 0; j < airport->checkInStaffList->Size(); j++){
		C = (CheckIn*)airport->checkInStaffList->Remove();
		
		airport->checkInStaffList->Append((void *)C);
		
		if(!airport->finalCheckin[C->getID()]){
			airport->checkinManagerLock->Acquire();

			airport->RequestingCheckinData[C->getID()] = true;
			airport->checkinBreakCV[C->getID()]->Signal(airport->checkinLock[C->getID()]);
			airport->checkinManagerCV->Wait(airport->checkinManagerLock);

			//Waits for the signal of corresponding Liaison
			airport->checkinLock[C->getID()]->Acquire();
			//Records the number of passengers per airline and stores into an array
			checkinPassengerCount[C->getAirline()] += C->getPassengers();
			checkinBaggageWeight[C->getAirline()] += C->getLuggageWeight();

			//Signals liaison that all the data has been collected
			airport->checkinCV[C->getID()]->Signal(airport->checkinLock[C->getID()]);
			airport->checkinLock[C->getID()]->Release();
        }

	}
}

void Manager::CargoRequest(Cargo *CH){
	for(int i = 0; i < airport->numAirlines; i++){			//prevents multicounting 
    	cargoHandlersBaggageCount[i] = 0;
    	cargoHandlersBaggageWeight[i] = 0;
    }
	for(int i = 0; i < airport->cargoHandlerList->Size(); i++){

		if(airport->cargoState[i] == C_BREAK){
			airport->CargoHandlerManagerLock->Acquire();
			CH = (Cargo*)airport->cargoHandlerList->Remove();
			airport->cargoHandlerList->Append((void *)CH);
			airport->RequestingCargoData[CH->getID()] = true;
			// airport->cargoCV->Broadcast(airport->cargoLock[i]);
			airport->cargoDataCV[i]->Signal(airport->cargoLock[i]);
			airport->cargoManagerCV[CH->getID()]->Wait(airport->CargoHandlerManagerLock);
			
			//Waits for the signal of corresponding Liaison
			airport->cargoDataLock[CH->getID()]->Acquire();
			//Records the total weight per airline and stores into an array
			for(int k = 0; k < airport->numAirlines; k++){
				cargoHandlersBaggageWeight[k] += CH->getWeight(k);
				cargoHandlersBaggageCount[k] += CH->getLuggage(k);
			}
			//Signals liaison that all the data has been collected
			airport->cargoDataCV[CH->getID()]->Signal(airport->cargoDataLock[CH->getID()]);
			airport->cargoDataLock[CH->getID()]->Release();
		}

	}
}

void Manager::SecurityDataRequest() {
	airport->updateClearCount->Acquire();
	securityInspectorPassengerCount = airport->clearPassengerCount;
	airport->updateClearCount->Release();
}
