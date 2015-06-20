#include "manager.h"

Manager::Manager(Airport* airport_) {
	airport = airport_;
	airlineLuggage = new List*[airport->numAirlines];
	liaisonBaggageCount = new int[airport->numAirlines];
	cargoHandlersBaggageWeight = new int[airport->numAirlines];
	cargoHandlersBaggageCount = new int[airport->numAirlines];
	checkinBaggageWeight = new int[airport->numAirlines];
	liaisonPassengerCount = new int[airport->numAirlines];
	checkinPassengerCount = new int[airport->numAirlines];
	securityInspectorPassengerCount = new int[airport->numAirlines];
	clearAirline = new bool[airport->numAirlines];
	for (int i = 0; i < (airport->numAirlines); i++) {
		clearAirline[i] = false;
		checkinPassengerCount[i] = 0;
		checkinBaggageWeight[i] = 0;
	}
	clearAirlineCount = 0;

	//Prevent Garbage values
}

Manager::~Manager() {
	delete[] airlineLuggage;
	delete[] liaisonBaggageCount;
	delete[] cargoHandlersBaggageWeight;
	delete[] cargoHandlersBaggageCount;
	delete[] checkinBaggageWeight;
	delete[] liaisonPassengerCount;
	delete[] checkinPassengerCount;
	delete[] securityInspectorPassengerCount;
}

void Manager::PrintCount()
{
    printf("\n");
    // Total passenger statistics
    int totalLiaisonPassengers  = 0;
    int totalCheckinPassengers  = 0;
    int totalSecurityPassengers = 0;
    for (int a = 0; a < airport->numAirlines; a++)
    {
        totalLiaisonPassengers  += liaisonPassengerCount[a];
        totalCheckinPassengers  += checkinPassengerCount[a];
        totalSecurityPassengers += securityInspectorPassengerCount[a];
    }
    printf("Passenger count reported by airport liaison = %d\n",        totalLiaisonPassengers  );
    printf("Passenger count reported by airline check-in staff = %d\n", totalCheckinPassengers  );
    printf("Passenger count reported by security inspector = %d\n",     totalSecurityPassengers );
    
    // Baggage statistics for each airline
    for (int a = 0; a < airport->numAirlines; a++)
    {
        printf("From setup: Baggage count of airline %d = %d\n",                   a, airport->airlines[a]->totalBagCount  );
        printf("From airport liaison: Baggage count of airline %d = %d\n",         a, liaisonBaggageCount[a]               );
        printf("From cargo handlers: Baggage count of airline %d = %d\n",          a, cargoHandlersBaggageCount[a]         );
        printf("From setup: Baggage weight of airline %d = %d\n",                  a, airport->airlines[a]->totalBagWeight );
        printf("From airline check-in staff: Baggage weight of airline %d = %d\n", a, checkinBaggageWeight[a]              );
        printf("From cargo handlers: Baggage weight of airline %d = %d\n",         a, cargoHandlersBaggageWeight[a]        );
    }
}

void Manager::MakeRounds() {
	bool cargoHandlersOnBreak = false;
	bool liaisonDone = false;
	bool CargoDone = false;
	bool CheckinDone = false;
	bool ready = true;
	int counter = 0;
	Liaison* L = NULL;
	Cargo* CH = NULL;
	CheckIn* C = NULL;
	SecurityInspector* SI = NULL;
	while (true) {
		//keeps track of how many cargo handlers are on break

		//Cargo handler interaction

		airport->conveyorLock->Acquire();
		if (!airport->conveyor->IsEmpty()) {

			counter = 0;
			for (int i = 0; i < airport->cargoHandlerList->Size(); i++) {
				if (airport->cargoState[i] == C_BREAK) {
					counter++;
				}
			}
			//If all handlers are on break, broadcast
			if (counter == airport->cargoHandlerList->Size()) {
				printf("Airport manager calls back all the cargo handlers from break\n");

				for (int i = 0; i < airport->cargoHandlerList->Size(); i++) {
					airport->cargoLock[i]->Acquire();
					airport->cargoDataCV[i]->Signal(airport->cargoLock[i]);

				}
				for (int i = 0; i < airport->cargoHandlerList->Size(); i++) {
					airport->cargoLock[i]->Release();
				}

			}
		}
		airport->conveyorLock->Release();

		//These functions gather data from each of the thread classes
		LiaisonDataRequest(L);

		CargoRequest(CH);

		CheckinDataRequest(C);

		SecurityDataRequest(SI);

		// check for boarding announcement
		for (int a = 0; a < airport->numAirlines; a++) {
			if (!clearAirline[a]) {
				airport->airlineLock[a]->Acquire();
                //printf("manager: checking airline %d; pass cleared = %d, tickets issued = %d; bags loaded = %d, bag count = %d\n", a, securityInspectorPassengerCount[a], airport->airlines[a]->ticketsIssued, airport->aircraft[a]->Size(), airport->airlines[a]->totalBagCount);
                //PrintCount();
				if (securityInspectorPassengerCount[a] >= airport->airlines[a]->ticketsIssued
					&& airport->aircraft[a]->Size()    >= airport->airlines[a]->totalBagCount) {
					printf("Airport manager gives a boarding call to airline %d\n", a);
					airport->boardingLock[a]->Acquire();
					airport->boardingCV[a]->Broadcast(airport->boardingLock[a]);
					clearAirline[a] = true;
					clearAirlineCount++;
					airport->boardingLock[a]->Release();
				}
				airport->airlineLock[a]->Release();
			}
		}
		if (clearAirlineCount == airport->numAirlines) {
            
            PrintCount();
            
			currentThread->Finish();
		}

		for (int i = 0; i < 50; i++) //this makes the manager give up the CPU otherwise he would hog the CPU
			currentThread->Yield();

	}
}

void Manager::LiaisonDataRequest(Liaison *L) {

	//Gather data from liaisons
	for (int i = 0; i < airport->numAirlines; i++) {	//prevents multicounting
		liaisonPassengerCount[i] = 0;
		liaisonBaggageCount[i] = 0;
	}
	for (int j = 0; j < airport->liaisonList->Size(); j++) {
		L = (Liaison*) airport->liaisonList->Remove();

		airport->liaisonList->Append((void *) L);

		if (airport->liaisonState[j] == L_FREE) {
			airport->liaisonManagerLock->Acquire();

			L = (Liaison*) airport->liaisonList->Remove();

			airport->liaisonList->Append((void *) L);
			airport->RequestingLiaisonData[L->getID()] = true;
			airport->liaisonCV[L->getID()]->Signal(
					airport->liaisonLock[L->getID()]);

			airport->liaisonManagerCV->Wait(airport->liaisonManagerLock);
			//Waits for the signal of corresponding Liaison
			airport->liaisonLock[L->getID()]->Acquire();

			//Records the number of passengers per airline and stores into an array
			for (int k = 0; k < airport->numAirlines; k++) {
				liaisonPassengerCount[k] += L->getPassengers(k);
				liaisonBaggageCount[k] += L->getLuggageCount(k);
			}

			//Signals liaison that all the data has been collected
			airport->liaisonCV[L->getID()]->Signal(
					airport->liaisonLock[L->getID()]);
			airport->liaisonLock[L->getID()]->Release();
		}
	}
}

void Manager::CheckinDataRequest(CheckIn *C) {
	//Gather data from check-in staff: special case, because check-in can close
	int* newCheckinBaggageWeight  = new int[airport->numAirlines];
	int* newCheckinPassengerCount = new int[airport->numAirlines];
	for (int i = 0; i < airport->numAirlines; i++)
    {
		newCheckinPassengerCount[i] = 0;
		newCheckinBaggageWeight[i] = 0;
	}

	for (int j = 0; j < airport->checkInStaffList->Size(); j++) {
		C = (CheckIn*) airport->checkInStaffList->Remove();

		airport->checkInStaffList->Append((void *) C);

		if (!airport->finalCheckin[C->getID()]) {
			airport->checkinManagerLock->Acquire();

			airport->RequestingCheckinData[C->getID()] = true;
			airport->checkinBreakCV[C->getID()]->Signal(
					airport->checkinLock[C->getID()]);
			airport->checkinManagerCV->Wait(airport->checkinManagerLock);

			//Waits for the signal of corresponding CheckIn
			airport->checkinLock[C->getID()]->Acquire();
			//Records the number of passengers per airline and stores into an array
			newCheckinPassengerCount[C->getAirline()] += C->getPassengers();
			newCheckinBaggageWeight[C->getAirline()]  += C->getLuggageWeight();
            
			//Signals checkin that all the data has been collected
			airport->checkinCV[C->getID()]->Signal(
					airport->checkinLock[C->getID()]);
			airport->checkinLock[C->getID()]->Release();
		}

	}
	for (int i = 0; i < airport->numAirlines; i++)
    {
		if (newCheckinPassengerCount[i] > checkinPassengerCount[i]) checkinPassengerCount[i] = newCheckinPassengerCount[i];
		if (newCheckinBaggageWeight[i]  > checkinBaggageWeight[i])  checkinBaggageWeight[i]  = newCheckinBaggageWeight[i];
	}
}

void Manager::CargoRequest(Cargo *CH) {
	for (int i = 0; i < airport->numAirlines; i++) {	//prevents multicounting
		cargoHandlersBaggageCount[i] = 0;
		cargoHandlersBaggageWeight[i] = 0;
	}

	int cargoNum = airport->cargoHandlerList->Size();

	for (int i = 0; i < cargoNum; i++) {

		//if (airport->cargoState[i] == C_BREAK) {
			airport->CargoHandlerManagerLock->Acquire();
			CH = (Cargo*) airport->cargoHandlerList->Remove();
			airport->cargoHandlerList->Append((void *) CH);
			airport->RequestingCargoData[CH->getID()] = true;
			airport->cargoDataCV[i]->Signal(airport->cargoLock[i]);
			airport->cargoManagerCV[CH->getID()]->Wait(
					airport->CargoHandlerManagerLock);

			//Waits for the signal of corresponding Cargo
			airport->cargoDataLock[CH->getID()]->Acquire();
			//Records the total weight per airline and stores into an array
			for (int k = 0; k < airport->numAirlines; k++) {
				//printf("*****updating cargo count for cargohandler %d*********\n",i);
				cargoHandlersBaggageWeight[k] += CH->getWeight(k);
				cargoHandlersBaggageCount[k] += CH->getLuggage(k);
			}
			//Signals cargo that all the data has been collected
			airport->cargoDataCV[CH->getID()]->Signal(
					airport->cargoDataLock[CH->getID()]);
			airport->cargoDataLock[CH->getID()]->Release();
		//}

	}
}

void Manager::SecurityDataRequest(SecurityInspector *SI) {
	airport->securityQueuesLock->Acquire();

	//reset to 0
	for (int i = 0; i < (airport->numAirlines); i++) {
		securityInspectorPassengerCount[i] = 0;
	}

	SecurityInspector* si;
	int securityNum = airport->securityInspectorList->Size();
	//gather clear passenger count from all security inspector
	for (int i = 0; i < securityNum; i++) {
		si = (SecurityInspector*) airport->securityInspectorList->Remove();
		int* count = si->getClearCount();
		for (int j = 0; j < (airport->numAirlines); j++) {
			securityInspectorPassengerCount[j] += count[j];
		}
		airport->securityInspectorList->Append(si);
	}

	airport->securityQueuesLock->Release();
}
